#ifndef _MAIN_H
#define _MAIN_H

#include <Arduino.h>

#include "magnet.h"
#include "stepper.h"
#include "system.h"
#include "datasheet.h"

// 初始化函数
void Pin_Mode_Init();
void Serial_Reader(void *pvParameters);
void Instruction_Executant(void *pvParameters);

// 系统控制函数
void System_Relax(bool isFromPC);
void System_Start();
void System_Reset(bool isFromPC);

void PWM_Sender();

#endif