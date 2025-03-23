#include "stepper.h"

// 根据指令设置电机方向并发送脉冲
// 3: 顺时针轻微转动
// 4: 逆时针轻微转动
// 5: 连续3圈（360度）
// 6: 顺时针90度
// 7: 逆时针90度
// 8: 180度
void Stepper_Control(int id, int op) {

    int pin_dir = (id == 1 ? STEPPER_L_DIR : STEPPER_R_DIR);
    int pin_pul = (id == 1 ? STEPPER_L_PUL : STEPPER_R_PUL);
    int *degree = (id == 1 ? &robot.l.degree : &robot.r.degree);

    switch (op) {
    case 3: case 4:
        digitalWrite(pin_dir, (op == 3 ? HIGH : LOW));
        Pulse_Sender(pin_pul, DELTA_PULSE);
        break;
    case 5:
        digitalWrite(pin_dir, (*degree > 0 ? LOW : HIGH));
        Pulse_Sender(pin_pul, PULSE360);
        *degree += 360;
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
        *degree += 180 * (*degree > 0 ? -1 : 1);
        break;
    default:
        return;
    }
    // Serial.println(robot.l.degree);
    // Serial.println(robot.r.degree);
}

void Stepper_Position_Init() {
    // 校正电机方向
    digitalWrite(STEPPER_L_DIR, HIGH);
    Pulse_Sender(STEPPER_L_PUL, PULSE360 / 8);
    digitalWrite(STEPPER_L_DIR, LOW);

    while (!digitalRead(SENSOR_L_PIN)) {
        Pulse_Sender(STEPPER_L_PUL, 1);
        delay(ReverseSpeed);
    }
    Pulse_Sender(STEPPER_L_PUL, stepperLcorrection);
    // stepperLcorrection 是微调参数
    digitalWrite(STEPPER_R_DIR, LOW);
    Pulse_Sender(STEPPER_R_PUL, PULSE360 / 8);
    digitalWrite(STEPPER_R_DIR, HIGH);

    while (!digitalRead(SENSOR_R_PIN)) {
        Pulse_Sender(STEPPER_R_PUL, 1);
        delay(ReverseSpeed);
    }

    Pulse_Sender(STEPPER_R_PUL, stepperRcorrection);
    robot.isReady = true;
    return;
}


// 对于 smoothstep 函数，设定一个简单模型：
// v(t) = v_max * (3*(t/T)^2 - 2*(t/T)^3)
// 则位移： x(t) = ∫0^t v(τ)dτ = v_max*(t^3/T^2 - t^4/(2*T^3))
// 每跨过 1 步，则产生一个脉冲



double acc_frenquence[MAX_STEPS];
int time_delay[MAX_STEPS];
int DriveTimeDelay[MAX_STEPS];
int TwistTimeDelay[MAX_STEPS];
int RaceTimeDelay[MAX_STEPS];

// 预先计算加速阶段中每个步进脉冲应触发的时间

// 用来存储预计算的脉冲触发时刻（单位：微秒）
double stepTimes[4][MAX_STEPS];
int numSteps[4];
// 模拟连续时间，步长 dt（秒）；dt 越小，采样越精细
const double dt = 0.00000001;
void generateSCurveStepTimes(double *stepTimes, int& numSteps, int pulse_x, double T_mid)
{
    double v_max = 2 * pulse_x / T_mid;
    double t = 0.0, lastT = 0;
    double lastStepPos = 0.0, pos;
    numSteps = 0;

    // 当 t 超过加速时间或步数达到上限时停止计算
    while (numSteps < pulse_x)
    {
        // 计算当前位移 x(t)
        // 使用 smoothstep 模型： x(t) = v_max * (t^3/T_accel^2 - t^4/(2*T_accel^3))
        pos = v_max * ((t * t * t) / (T_mid * T_mid) - (t * t * t * t) / (2 * T_mid * T_mid * T_mid));

        // 当累计位移跨过下一个整数（即步数）时，记录该时刻
        if (floor(pos) > lastStepPos) {
            stepTimes[numSteps++] = ((t-lastT) * 1000000.0); // 转换为微秒
            lastStepPos = floor(pos);
            lastT = t;
        }
        t += dt;
    }
}
void Stepper_Acc_Init() {
    generateSCurveStepTimes(stepTimes[0], numSteps[0], Pulse_Acc, 0.1 / 2);
    generateSCurveStepTimes(stepTimes[1], numSteps[1], Pulse_Acc, 0.1 / 2);
    generateSCurveStepTimes(stepTimes[2], numSteps[2], Pulse_Acc, 0.1 / 2);
    //Stepper_Acc_Init_Old();
}


// 初始化电机加速参数，包括频率和时间延迟
// void Stepper_Acc_Init_Old() {
//     int num = Pulse_Acc / 2; // 计算加速曲线的中点
//     // 延时
//     for (int i = 1; i <= Pulse_Acc; i++)
//     {
//         acc_frenquence[i] = Initial_Frequence + (double)(End_Frequence - Initial_Frequence) / (double)(1.0 + exp(((double)acc_factor * (num - i) / num)));
//         time_delay[i] = round((1000000 / acc_frenquence[i]) / 2.0);
//     }
//     // 驱动
//     for (int i = 1; i <= Pulse_Acc; i++)
//     {
//         acc_frenquence[i] = DriveIniFreq + (double)(DriveEndFreq - DriveIniFreq) / (double)(1.0 + exp(((double)acc_factor * (num - i) / num)));
//         DriveTimeDelay[i] = round((1000000 / acc_frenquence[i]) / 2.0);
//     }
//     // 拧动
//     for (int i = 1; i <= Pulse_Acc; i++)
//     {
//         acc_frenquence[i] = TwistIniFreq + (double)(TwistEndFreq - TwistIniFreq) / (double)(1.0 + exp(((double)acc_factor * (num - i) / num)));
//         TwistTimeDelay[i] = round((1000000 / acc_frenquence[i]) / 2.0);
//     }
//     // 空转
//     for (int i = 1; i <= Pulse_Acc; i++)
//     {
//         acc_frenquence[i] = RaceIniFreq + (double)(RaceEndFreq - DriveIniFreq) / (double)(1.0 + exp(((double)acc_factor * (num - i) / num)));
//         RaceTimeDelay[i] = round((1000000 / acc_frenquence[i]) / 2.0);
//     }
//     return;
// }

void Pulse_Sender(int pin, int num) {
    if (num >= 2 * Pulse_Acc) 
    {
        int id = (pin == STEPPER_L_PUL ? \
            robot.l.isTight*(robot.r.isTight + 1) : robot.r.isTight * (robot.l.isTight + 1));
        

        // if (pin == STEPPER_L_PUL)
        // {
        //     if (robot.l.isTight == 0) // 左爪空转
        //         ChosenMode = RaceTimeDelay;
        //     else if (robot.r.isTight == 1) // 拧动
        //         ChosenMode = TwistTimeDelay;
        //     else if (robot.r.isTight == 0) // 带转动
        //         ChosenMode = DriveTimeDelay;
        //     else
        //         ChosenMode = time_delay;
        // }
        // else
        // {
        //     if (robot.r.isTight == 0)
        //         ChosenMode = RaceTimeDelay;
        //     else if (robot.l.isTight == 1)
        //         ChosenMode = TwistTimeDelay;
        //     else if (robot.l.isTight == 0)
        //         ChosenMode = DriveTimeDelay;
        //     else
        //         ChosenMode = time_delay;
        // }

        for (int i = 1; i <= Pulse_Acc; i++)
            PULSE_GENERATOR(pin, stepTimes[id][i]);
        for (int i = 1; i <= num - 2 * Pulse_Acc; i++)
            PULSE_GENERATOR(pin, stepTimes[id][Pulse_Acc]);
        for (int i = Pulse_Acc; i >= 1; i--)
            PULSE_GENERATOR(pin, stepTimes[id][i]);
    }
    else {
        for (int i = 1; i <= num; i++)
            PULSE_GENERATOR(pin, SlightDelay);
    }
}
