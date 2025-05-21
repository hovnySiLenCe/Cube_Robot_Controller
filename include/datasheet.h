#ifndef _DATASHEET_H_
#define _DATASHEET_H_

#include <system.h>
#include "stepper.h"

// -------------------------------
#define MAGNET_TIGHT_DELAY_ID 0*10+0
#define MAGNET_LOOSE_DELAY_ID 0*10+1

#define CONTINUOUS_TWIST_DELAY_ID 0*10+2
#define STEPPER_REVERSE_DELAY_ID 0*10+3
#define STEPPER_DEBUG_DELAY_ID 0*10+4

#define MAGNET_TIGHT_RATIO_DELAY_ID 0*10+5
#define MAGNET_LOOSE_RATIO_DELAY_ID 0*10+6
#define MAGNET_DELTA_DELAY_ID 0*10+7

// ---------------------------------
#define STEPPER_L_CORRECTION_ID 1*10+0
#define STEPPER_R_CORRECTION_ID 1*10+1
#define PULSE_90_ID 1*10+2
#define PULSE_180_ID 1*10+3
#define PULSE_360_ID 1*10+4
#define DELTA_PULSE_ID 1*10+5

// -----------------------------

#define ACC_TIME_OF_RACE_ID 2*10+0
#define ACC_SPEED_OF_RACE_ID 2*10+1
#define ACC_TIME_OF_TURN_ID 2*10+2
#define ACC_SPEED_OF_TURN_ID 2*10+3
#define ACC_TIME_OF_TWIST_ID 2*10+4
#define ACC_SPEED_OF_TWIST_ID 2*10+5
#define ACC_TIME_OF_DEBUG_ID 2*10+6
#define ACC_SPEED_OF_DEBUG_ID 2*10+7

#define MAX_KEY_NUM 30

struct Data_Sheet_t {
    unsigned short key[MAX_KEY_NUM]; // 数据表结构体
    void Show() {
        for (int i = 0; i < MAX_KEY_NUM; i++) {
            Serial.printf("%04d ",  key[i]);
            if(i%10 == 9) Serial.println();
        }
    }
    void Merge(const Data_Sheet_t& x) {
        for (int i = 0; i < MAX_KEY_NUM; i++) {
            if(!x.key[i]) continue;
            key[i] = x.key[i];
        }
    }
    Data_Sheet_t() {
    key[MAGNET_TIGHT_DELAY_ID] = 50; // 单位ms
    key[MAGNET_LOOSE_DELAY_ID] = 111;

    key[MAGNET_TIGHT_RATIO_DELAY_ID] = 5; // 单位ms
    key[MAGNET_LOOSE_RATIO_DELAY_ID] = 15;

    key[MAGNET_DELTA_DELAY_ID] = 5;
    key[CONTINUOUS_TWIST_DELAY_ID] = 10; // 连续拧动的延迟，单位ms

    key[STEPPER_REVERSE_DELAY_ID] = 5000; // 初始位置延迟，单位us
    key[STEPPER_DEBUG_DELAY_ID] = 5000; // 轻微延迟，单位us

    // ----------- 脉冲数设置 -----------
    key[STEPPER_L_CORRECTION_ID] = 39; // 补偿脉冲
    key[STEPPER_R_CORRECTION_ID] = 42;

    key[PULSE_360_ID] = 2000; // 360度转动的脉冲数
    key[PULSE_180_ID] = 1000; // 180度转动的脉冲数
    key[PULSE_90_ID] = 500;   // 90度转动的脉冲数
    key[DELTA_PULSE_ID] = 1;  // 轻微转动的脉冲数

    // S型曲线加速度步数
    key[ACC_TIME_OF_RACE_ID] = 5;
    key[ACC_SPEED_OF_RACE_ID] = 5000;
    key[ACC_TIME_OF_TURN_ID] = 15;
    key[ACC_SPEED_OF_TURN_ID] = 200;
    key[ACC_TIME_OF_TWIST_ID] = 10;
    key[ACC_SPEED_OF_TWIST_ID] = 3000;
    key[ACC_TIME_OF_DEBUG_ID] = 30;
    key[ACC_SPEED_OF_DEBUG_ID] = 5000;
    }
};

extern Data_Sheet_t dsheet;

void Data_Sheet_Init();
void Data_Sheet_Read();
void Data_Sheet_Modify(int operation);
void Data_Sheet_Save();

#endif