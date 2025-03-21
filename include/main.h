#ifndef _MAIN_H
#define _MAIN_H

#include <Arduino.h>

#include "magnet.h"
#include "stepper.h"
#include "system.h"

// 初始化函数
void Pin_Mode_Init();
void Serial_Reader(void *pvParameters);
void Instruction_Executant(void *pvParameters);

// 系统控制函数
void System_Relax();
void System_Start();
void System_Reset();

void PWM_Sender();

//void Stepper_Acc_Init();
//void MotorSetAndPulse(int id, int dat);
//void Pulse_Sender(int pin, int num);
//void Stepper_Position_Init();

// 阀门控制函数
//void ValveSet(int id, int dat);
//void valveLTight();
// void valveLOpen();
//void valveRTight();
// void valveROpen();
//void valveLLoose();
//void valveRLoose();
// void valveLSlightTight();
// void valveLSlightOpen();
// void valveRSlightTight();
// void valveRSlightOpen();
//void Valve_All_Loose();
//void Valve_All_Tight();
// void Valve_All_Open();

#endif