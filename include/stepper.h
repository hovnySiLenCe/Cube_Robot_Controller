#ifndef _STEPPER_H_
#define _STEPPER_H_

#include "system.h"
#include <Preferences.h>

#define L_STEPPER_ON() digitalWrite(STEPPER_L_ENA,LOW)
#define L_STEPPER_OFF() digitalWrite(STEPPER_L_ENA,HIGH)
#define R_STEPPER_ON() digitalWrite(STEPPER_R_ENA,LOW)
#define R_STEPPER_OFF() digitalWrite(STEPPER_R_ENA,HIGH)

#define STEPPER_ALL_ON() do { L_STEPPER_ON();R_STEPPER_ON(); } while(0)
#define STEPPER_ALL_OFF() do { L_STEPPER_OFF();R_STEPPER_OFF(); } while(0)

#define PULSE_GENERATOR(pin, delta_time) \
    do{\
        digitalWrite(pin, HIGH);\
        delayMicroseconds(delta_time);\
        digitalWrite(pin, LOW);\
        delayMicroseconds(delta_time);\
    } while(0)

#define MAX_STEPS 203

void Stepper_Control(int id, int op);
void Stepper_Position_Init();
void Stepper_Acc_Init();
void Pulse_Sender(int pin,int num);

#endif