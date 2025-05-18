#include "stepper.h"

bool ValidityCheck(int id) {
    int degree = (id == 3 ? robot.l.degree : robot.r.degree);
    return abs(degree % 180) != 90;
}
// 根据指令设置电机方向并发送脉冲
// 3: 顺时针轻微转动
// 4: 逆时针轻微转动
// 5: 360度
// 6: 顺时针90度
// 7: 逆时针90度
// 8: 180度
void Stepper_Control(int id, int op) {

    int pin_dir = (id == 1 ? STEPPER_L_DIR : STEPPER_R_DIR);
    int pin_pul = (id == 1 ? STEPPER_L_PUL : STEPPER_R_PUL);
    int *degree = (id == 1 ? &robot.l.degree : &robot.r.degree);

    if(!ValidityCheck(id)) {
        Serial.println("\b [ERROR] Command Illegal!");
        return;
    }

    switch (op) {
    case 3: case 4:
        Serial.println(((op == 3) ? "CW 1 pulse" : "ACW 1 pulse"));
        digitalWrite(pin_dir, (op == 3 ? CW : ACW));
        Pulse_Sender(pin_pul, DELTA_PULSE);
        break;
    case 5:
        Serial.println((*degree > 0 ? -360 : 360));
        digitalWrite(pin_dir, (*degree > 0 ? ACW : CW));
        Pulse_Sender(pin_pul, PULSE360);
        *degree += (*degree > 0 ? -360 : 360);
        break;
    case 6: 
        Serial.println(90);
        digitalWrite(pin_dir, CW);
        Pulse_Sender(pin_pul, PULSE90);
        *degree += 90;
        break;
    case 7:
        Serial.println(-90);
        digitalWrite(pin_dir, ACW);
        Pulse_Sender(pin_pul, PULSE90);
        *degree -= 90;
        break;
    case 8:
        Serial.println((*degree > 0 ? -180 : 180));
        digitalWrite(pin_dir, (*degree > 0 ? ACW : CW));
        Pulse_Sender(pin_pul, PULSE180);
        *degree += (*degree > 0 ? -180 : 180);
        break;
    default:
        Serial.println(" Invalid Command");
        return;
    }
    // Serial.println(robot.l.degree);
    // Serial.println(robot.r.degree);
}

void Stepper_Position_Init() {
    Serial.println("---------- Initializing Stepper Position ----------");
    // 校正电机方向
    digitalWrite(STEPPER_L_DIR, CW);

    while (digitalRead(SENSOR_L_PIN)) {
        PULSE_GENERATOR(STEPPER_L_PUL, STEPPER_REVERSE_DELAY);
    }
    digitalWrite(STEPPER_L_DIR, ACW);
    Pulse_Sender(STEPPER_L_PUL, PULSE360/8-stepperLcorrection);
    Serial.println("[SUCCESS] L_Stepper_Initialized");

    // stepperLcorrection 是微调参数
    digitalWrite(STEPPER_R_DIR, CW);

    while (digitalRead(SENSOR_R_PIN)) {
        PULSE_GENERATOR(STEPPER_R_PUL, STEPPER_REVERSE_DELAY);
    }
    digitalWrite(STEPPER_R_DIR, ACW);
    Pulse_Sender(STEPPER_R_PUL, PULSE360/8-stepperRcorrection);
    Serial.println("[SUCCESS] R_Stepper_Initialized");
    robot.isReady = true;
    return;
}


// 对于 smoothstep 函数，设定一个简单模型：
// v(t) = v_max * (3*(t/T)^2 - 2*(t/T)^3)
// 则位移： x(t) = ∫0^t v(τ)dτ = v_max*(t^3/T^2 - t^4/(2*T^3))
// 每跨过 1 步，则产生一个脉冲


// double acc_frenquence[MAX_STEPS];
// int time_delay[MAX_STEPS];
// int DriveTimeDelay[MAX_STEPS];
// int TwistTimeDelay[MAX_STEPS];
// int RaceTimeDelay[MAX_STEPS];

#define MAX_DELAY_SEQUENCE 4
#define RACE_ID 0
#define TURN_ID 1
#define TWIST_ID 2
#define DEBUG_ID 3


// 定义用来存储S型曲线的结构体
struct Acc_Array_t {
    int accPulse, T_mid;
    int stepTimes[MAX_STEPS];
    void Info() {
        Serial.printf("accPulse = %d, T_mid = %d\n", accPulse, T_mid);
        // for(int i = 0; i < accPulse; i++) {
        //     Serial.printf("%5d ", stepTimes[i]);
        //     if(i % 10 == 9) Serial.println();
        // }
    }
    Acc_Array_t(int pulse_x, int T_mid) : accPulse(pulse_x), T_mid(T_mid) { memset(stepTimes, 0, sizeof(stepTimes)); } // 构造函数初始化
    Acc_Array_t() {}
    bool operator==(const Acc_Array_t& x) const {
        return accPulse == x.accPulse && T_mid == x.T_mid;
    }
}accArrays[MAX_DELAY_SEQUENCE];

// 模拟连续时间，步长 dt（秒）；dt 越小，采样越精细
const double dt = 0.0001;
bool generateSCurveStepTimes(Acc_Array_t* acc_p, int speed_x, double T_mid) // T_mid 单位为ms
{
    Serial.printf("[INFO] Generating accArrays, pulse_x = %d sps, T_mid = %d ms\n", speed_x, T_mid);
    if(*acc_p == Acc_Array_t(speed_x, T_mid)) return true; // 如果数据相同，则不重新计算
    int pulse_x = (int)(speed_x * T_mid / 1000.0); // 计算脉冲数
    double v_max = 2 * pulse_x / T_mid;
    double t = 0.0, lastT = 0.0;
    double lastStepPos = 0.0, pos;
    int numSteps = 0;

    acc_p->T_mid = T_mid * 1000; // 单位为us
    acc_p->accPulse = pulse_x;

    // 当 t 超过加速时间或步数达到上限时停止计算
    while (numSteps < pulse_x)
    {
        // 计算当前位移 x(t)
        // 使用 smoothstep 模型： x(t) = v_max * (t^3/T_accel^2 - t^4/(2*T_accel^3))
        pos = v_max * ((t * t * t) / (T_mid * T_mid) - (t * t * t * t) / (2 * T_mid * T_mid * T_mid));
        
        // 当累计位移跨过下一个整数（即步数）时，记录该时刻
        if (floor(pos) > lastStepPos) {
            acc_p->stepTimes[numSteps++] = (t-lastT) * 1000 / 2; // 转换为us
            lastStepPos = floor(pos);
            lastT = t;
        }
        t += dt;
    }
    return false; // 返回 false 表示数据已更新
}

bool reGenerateSCurveStepTimes() {
    Serial.println("[INFO] Regenerating accArrays");
    bool isSame = true;
    isSame &= generateSCurveStepTimes(&accArrays[RACE_ID], ACC_SPEED_OF_RACE, ACC_TIME_OF_RACE);
    isSame &= generateSCurveStepTimes(&accArrays[TURN_ID], ACC_SPEED_OF_TURN, ACC_TIME_OF_TURN);
    isSame &= generateSCurveStepTimes(&accArrays[TWIST_ID], ACC_SPEED_OF_TWIST, ACC_TIME_OF_TWIST);
    isSame &= generateSCurveStepTimes(&accArrays[DEBUG_ID], ACC_SPEED_OF_DEBUG, ACC_TIME_OF_DEBUG);
    return isSame;
}

Preferences prefs; // 用于存储数据的对象
void Stepper_Acc_Init() {
    Serial.println("----- Initializing Stepper Acceleration Curve ------");
    if(!prefs.begin("stepper", false)) {
        Serial.println("[FATAL] Failed to initialize preferences");
        reGenerateSCurveStepTimes();
        Serial.println("[SUCCESS] Generated accArrays to flash");
        return;
    }
    if (prefs.isKey("accArrays")) {
        prefs.getBytes("accArrays", &accArrays, sizeof(accArrays));
        Serial.println("[SUCCESS] Loaded accArrays from flash");
        for(int i = 0; i < MAX_DELAY_SEQUENCE; i++)
            accArrays[i].Info();
        return;
    }

    if(reGenerateSCurveStepTimes()) {
        Serial.println("[INFO] No need to update accArrays");
        return;
    }

    prefs.putBytes("accArrays", &accArrays, sizeof(accArrays));
    Serial.println("[SUCCESS] Saved accArrays to flash");
}

void SaveAccArrays() {
    if(prefs.putBytes("accArrays", &accArrays, sizeof(accArrays)))
        Serial.println("[SUCCESS] Saved accArrays to flash");
    else
        Serial.println("[FATAL] Failed to save accArrays to flash");
}

void Pulse_Sender(int pin, int num) {

    int id = (pin == STEPPER_L_PUL) ? robot.l.isTight*((int)robot.r.isTight + 1) : robot.r.isTight * ((int)robot.l.isTight + 1);
    if(!robot.isReady) id = 3;
    int accPulse = accArrays[id].accPulse;
    int* stepTimes = accArrays[id].stepTimes;

    //Serial.printf(" -> : id = %d\n", id);
    //Serial.printf(" -> : Sending %d pulses to pin %d\n", num, pin);

    if (robot.isReady && num >= 2 * accPulse && !robot.isDebug) 
    {
        for (int i = 0; i < accPulse; i++)
            PULSE_GENERATOR(pin, stepTimes[i]);
        for (int i = 0; i < num - 2 * accPulse; i++)
            PULSE_GENERATOR(pin, stepTimes[accPulse-1]);
        for (int i = accPulse-1; i >= 0; i--)
            PULSE_GENERATOR(pin, stepTimes[i]);
    }
    else {
        for (int i = 1; i <= num; i++)
            PULSE_GENERATOR(pin, STEPPER_DEBUG_DELAY);
    }
}
