#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <Arduino.h>
#include <Preferences.h>

// ########## 引脚定义 #########
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
#define MAGNET_L_PIN 12
#define MAGNET_R_PIN 11

// 按钮引脚
#define BUTTOM_START_PIN 21
#define BUTTOM_RESET_PIN 48
#define BUTTOM_RELAX_PIN 38
#define BUTTOM_HAND_PIN 47

// 霍尔传感器引脚
#define SENSOR_L_PIN 2
#define SENSOR_R_PIN 1

// 蜂鸣器引脚
#define BUZZER_PIN 8

// ############# 参数定义 ############

// ----------- 延时设置 -----------

#define Magnet_Tight_Delay dsheet.key[MAGNET_TIGHT_DELAY_ID] // 单位ms
#define Magnet_Loose_Delay dsheet.key[MAGNET_LOOSE_DELAY_ID]

#define Magnet_Tight_Ratio_Delay dsheet.key[MAGNET_TIGHT_RATIO_DELAY_ID] // 单位ms
#define Magnet_Loose_Ratio_Delay dsheet.key[MAGNET_LOOSE_RATIO_DELAY_ID]

#define Magnet_Delta_Delay dsheet.key[MAGNET_DELTA_DELAY_ID]
#define Continous_Twist_Delay dsheet.key[CONTINUOUS_TWIST_DELAY_ID] // 连续拧动的延迟，单位ms

#define STEPPER_REVERSE_DELAY dsheet.key[STEPPER_REVERSE_DELAY_ID] // 初始位置延迟，单位us
#define STEPPER_DEBUG_DELAY dsheet.key[STEPPER_DEBUG_DELAY_ID] // 轻微延迟，单位us

// ----------- 脉冲数设置 -----------
// 补偿脉冲
#define stepperLcorrection dsheet.key[STEPPER_L_CORRECTION_ID] // 左电机补偿脉冲
#define stepperRcorrection dsheet.key[STEPPER_R_CORRECTION_ID]

#define PULSE360 dsheet.key[PULSE_360_ID] // 360度转动的脉冲数
#define PULSE180 dsheet.key[PULSE_180_ID] // 180度转动的脉冲数
#define PULSE90 dsheet.key[PULSE_90_ID]   // 90度转动的脉冲数
#define DELTA_PULSE dsheet.key[DELTA_PULSE_ID]    // 轻微转动的脉冲数

// S型曲线加速度步数
#define ACC_PULSE_OF_RACE dsheet.key[ACC_PULSE_OF_RACE_ID]
#define ACC_PULSE_OF_TURN dsheet.key[ACC_PULSE_OF_TURN_ID]
#define ACC_PULSE_OF_TWIST dsheet.key[ACC_PULSE_OF_TWIST_ID]
#define ACC_PULSE_OF_DEBUG dsheet.key[ACC_PULSE_OF_DEBUG_ID]

#define ACC_TIME_OF_RACE dsheet.key[ACC_TIME_OF_RACE_ID]
#define ACC_TIME_OF_TURN dsheet.key[ACC_TIME_OF_TURN_ID]
#define ACC_TIME_OF_TWIST dsheet.key[ACC_TIME_OF_TWIST_ID]
#define ACC_TIME_OF_DEBUG dsheet.key[ACC_TIME_OF_DEBUG_ID]


// ------------ 暂未使用的参数 -----------
#define acc_factor 5
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

class Hand_State_t {
public:
    bool isTight;
    int degree;
    Hand_State_t() : isTight(false), degree(0) {}
    ~Hand_State_t() {}
    void ResetState() {
        isTight =false;
        degree = 0;
    };
};

class Robot_Monitor_t {
private:
    /* data */
public:
    Hand_State_t l;
    Hand_State_t r;
    bool isReady, isDebug, preTwist, curTwist;
    Robot_Monitor_t() {};
    ~Robot_Monitor_t() {}
    void Init() {
        l.ResetState();
        r.ResetState();
        isReady = isDebug = false;
        preTwist = curTwist = false;
    }
    void HandConvert();
};

extern Robot_Monitor_t robot;

#endif