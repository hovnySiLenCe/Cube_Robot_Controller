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
        Serial.println("Command Illegal!");
        return;
    }

    switch (op) {
    case 3: case 4:
        digitalWrite(pin_dir, (op == 3 ? HIGH : LOW));
        Pulse_Sender(pin_pul, DELTA_PULSE);
        break;
    case 5:
        digitalWrite(pin_dir, (*degree > 0 ? LOW : HIGH));
        Pulse_Sender(pin_pul, PULSE360);
        *degree += (*degree > 0 ? -360 : 360);
        break;
    case 6: 
        digitalWrite(pin_dir, HIGH);
        Pulse_Sender(pin_pul, PULSE90);
        *degree += 90;
        break;
    case 7:
        digitalWrite(pin_dir, LOW);
        Pulse_Sender(pin_pul, PULSE90);
        *degree -= 90;
        break;
    case 8:
        digitalWrite(pin_dir, (*degree > 0 ? LOW : HIGH));
        Pulse_Sender(pin_pul, PULSE180);
        *degree += (*degree > 0 ? -180 : 180);
        break;
    default:
        return;
    }
    // Serial.println(robot.l.degree);
    // Serial.println(robot.r.degree);
}

#define CW HIGH
#define ACW LOW

void Stepper_Position_Init() {
    Serial.println("---------- Initializing Stepper Position ----------");
    // 校正电机方向
    digitalWrite(STEPPER_L_DIR, ACW);
    //Pulse_Sender(STEPPER_L_PUL, PULSE360 / 8);
    //digitalWrite(STEPPER_L_DIR, LOW);
    //delay(500);

    while (digitalRead(SENSOR_L_PIN)) {
        PULSE_GENERATOR(STEPPER_L_PUL, STEPPER_REVERSE_DELAY);
    }
    digitalWrite(STEPPER_L_DIR, CW);
    Pulse_Sender(STEPPER_L_PUL, PULSE360/8-stepperLcorrection);
    Serial.println("SUCCESS: L_Stepper_Initialized");

    // stepperLcorrection 是微调参数
    digitalWrite(STEPPER_R_DIR, ACW);
    //Pulse_Sender(STEPPER_R_PUL, PULSE360/8);
    while (digitalRead(SENSOR_R_PIN)) {
        PULSE_GENERATOR(STEPPER_R_PUL, STEPPER_REVERSE_DELAY);
        //Pulse_Sender(STEPPER_R_PUL, 1);
        //delay(ReverseSpeed);
    }
    digitalWrite(STEPPER_R_DIR, CW);
    Pulse_Sender(STEPPER_R_PUL, PULSE360/8-stepperRcorrection);
    Serial.println("SUCCESS: R_Stepper_Initialized");
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
    Acc_Array_t(int pulse_x, int T_mid) : accPulse(pulse_x), T_mid(T_mid) { memset(stepTimes, 0, sizeof(stepTimes)); } // 构造函数初始化
    Acc_Array_t() {}
    bool operator==(const Acc_Array_t& x) const {
        return accPulse == x.accPulse && T_mid == x.T_mid;
    }
}accArrays[MAX_DELAY_SEQUENCE];

// 模拟连续时间，步长 dt（秒）；dt 越小，采样越精细
const double dt = 0.000001;
bool generateSCurveStepTimes(Acc_Array_t* acc_p, int pulse_x, double T_mid)
{
    if(*acc_p == Acc_Array_t(pulse_x, T_mid)) return true; // 如果数据相同，则不重新计算
    double v_max = 2 * pulse_x / T_mid;
    double t = 0.0, lastT = 0.0;
    double lastStepPos = 0.0, pos;
    int numSteps = 0;

    acc_p->T_mid = T_mid*1e6; // 单位为微秒
    acc_p->accPulse = pulse_x;

    // 当 t 超过加速时间或步数达到上限时停止计算
    while (numSteps < pulse_x)
    {
        // 计算当前位移 x(t)
        // 使用 smoothstep 模型： x(t) = v_max * (t^3/T_accel^2 - t^4/(2*T_accel^3))
        pos = v_max * ((t * t * t) / (T_mid * T_mid) - (t * t * t * t) / (2 * T_mid * T_mid * T_mid));

        // 当累计位移跨过下一个整数（即步数）时，记录该时刻
        if (floor(pos) > lastStepPos) {
            acc_p->stepTimes[numSteps++] = ((t-lastT) * 1000000.0 / 2); // 转换为微秒
            lastStepPos = floor(pos);
            lastT = t;
        }
        t += dt;
    }
    return false; // 返回 false 表示数据已更新
}

Preferences prefs; // 用于存储数据的对象
void Stepper_Acc_Init() {
    Serial.println("----- Initializing Stepper Acceleration Curve ------");
    if(!prefs.begin("stepper", false)) {
        Serial.println("FETAL: Failed to initialize preferences");
        generateSCurveStepTimes(&accArrays[RACE_ID], ACC_PULSE_OF_RACE, 0.01 / 2);
        generateSCurveStepTimes(&accArrays[TURN_ID], ACC_PULSE_OF_TURN, 0.05 / 2);
        generateSCurveStepTimes(&accArrays[TWIST_ID], ACC_PULSE_OF_TWIST, 0.03 / 2);
        generateSCurveStepTimes(&accArrays[DEBUG_ID], ACC_PULSE_OF_DEBUG, 0.08 / 2);
        Serial.println("SUCCESS: Generated accArrays to flash");
        return;
    }
    if (prefs.isKey("accArrays")) {
        prefs.getBytes("accArrays", &accArrays, sizeof(accArrays));
        Serial.println("SUCCESS: Loaded accArrays from flash");
        return;
    }
    bool isSame = true;
    isSame &= generateSCurveStepTimes(&accArrays[RACE_ID], ACC_PULSE_OF_RACE, 0.01 / 2);
    isSame &= generateSCurveStepTimes(&accArrays[TURN_ID], ACC_PULSE_OF_TURN, 0.05 / 2);
    isSame &= generateSCurveStepTimes(&accArrays[TWIST_ID], ACC_PULSE_OF_TWIST, 0.03 / 2);
    isSame &= generateSCurveStepTimes(&accArrays[DEBUG_ID], ACC_PULSE_OF_DEBUG, 0.08 / 2);
    
    if(isSame) {
        Serial.println("-> : No need to update accArrays");
        return;
    }

    prefs.putBytes("accArrays", &accArrays, sizeof(accArrays));
    Serial.println("SUCCESS: Saved accArrays to flash");
}

void Pulse_Sender(int pin, int num) {

    int id = (pin == STEPPER_L_PUL) ? robot.l.isTight*((int)robot.r.isTight + 1) : robot.r.isTight * ((int)robot.l.isTight + 1);
    

    int accPulse = accArrays[id].accPulse;
    int* stepTimes = accArrays[id].stepTimes;

    if (robot.isReady && num >= 2 * accPulse) 
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
