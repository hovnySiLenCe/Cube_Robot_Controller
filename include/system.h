#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <Arduino.h>

// ---------- 引脚定义 ----------
// 左电机引脚
#define STEPPER_L_PUL 18 // 左电机脉冲引脚
#define STEPPER_L_DIR 17 // 左电机方向引脚
#define STEPPER_L_ENA 16 // 左电机使能引脚
#define STEPPER_L_ALM 15 // 左电机报警引脚

// 右电机引脚
#define STEPPER_R_PUL 7  // 右电机脉冲引脚
#define STEPPER_R_DIR 6  // 右电机方向引脚
#define STEPPER_R_ENA 5  // 右电机使能引脚
#define STEPPER_R_ALM 4  // 右电机报警引脚

// 电磁铁引脚
#define MAGNET_L_PIN 9
#define MAGNET_R_PIN 10

// 按钮引脚
#define BUTTOM_START_PIN 21
#define BUTTOM_RESET_PIN 48
#define BUTTOM_RELAX_PIN 38
#define BUTTOM_TIGHT_PIN 38//!!
#define BUTTOM_LOOSE_PIN 38//!!

// 霍尔传感器引脚
#define SENSOR_L_PIN 2
#define SENSOR_R_PIN 1

// 蜂鸣器引脚
#define BUZZER_PIN 8

// ---------- 参数定义 ----------

#define Magnet_Delay 111
#define Magnet_Delta_Delay 5


#define PULSE360 2000 // 360度转动的脉冲数
#define PULSE180 1000 // 180度转动的脉冲数
#define PULSE90 500   // 90度转动的脉冲数
#define DELTA_PULSE 1    // 轻微转动的脉冲数

#define ACC_PULSE 50
#define Initial_Frequence 7200  // 初始频率
#define End_Frequence 12000     // 结束频率

// 魔方驱动频率设定
#define DriveIniFreq 7200
#define DriveEndFreq 12000

// 魔方拧动频率设定
#define TwistIniFreq 9600
#define TwistEndFreq 15120

// 魔方空转频率设定
#define RaceIniFreq 13320
#define RaceEndFreq 18000

#define acc_factor 5
#define Continous_Twist_Delay 10 // 连续拧动的延迟
#define stepperLcorrection 12
#define stepperRcorrection 0
#define ReverseSpeed 10
#define SlightDelay 200 // 轻微延迟

typedef struct {
    bool isTight;
    int degree;
}Hand_State_t;

typedef struct {
    Hand_State_t l;
    Hand_State_t r;
    bool isReady, preTwist, curTwist;
    void Init() {
        l.isTight = r.isTight = false;
        l.degree = r.degree = 0;
        isReady = preTwist = curTwist = false;
    }
}Robot_Monitor_t;

extern Robot_Monitor_t robot;

#endif