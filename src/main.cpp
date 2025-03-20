#include "main.h"

/* 作者: LiBingle
 * 第二作者: Joshua
 * 第三作者：陇望遥
 * 版本: 2.0 fastest
 * 说明: 本程序为魔方机器人控制程序，通过串口接收指令控制电机和阀门的动作
 * 详细说明见代码注释
 */

// 初始化变量
QueueHandle_t instructions;
TaskHandle_t reader;
TaskHandle_t executant;

Robot_Monitor_t robot;
void setup() {
    // 初始化引脚
	Pin_Mode_Init();
    HAND_ALL_LOOSE(); // 松开所有电磁铁
    STEPPER_ALL_ON(); // 使能电机

    // 初始化电机位置和加速度参数
    Stepper_Acc_Init();
    Stepper_Position_Init();

    // 初始化监视器
    robot.Init();
    
	Serial.begin(115200); // 初始化串口

    // 初始化蜂鸣器
	digitalWrite(BUZZER_PIN, LOW);

	instructions = xQueueCreate(200, 9 * sizeof(char)); // 创建队列
	if (instructions == NULL) {
		Serial.println("{failed to create queue}");
	}
	xTaskCreatePinnedToCore(Serial_Reader, "Serial_Reader", 10000, NULL, 3, &reader, 0);
	delay(500);
	xTaskCreatePinnedToCore(Instruction_Executant, "Instruction_Executant", 10000, NULL, 3, &executant, 1);
    delay(500);
	Serial.println("{successfully initialized}");
}

void loop() {
  // 主循环代码，重复执行
}

void Serial_Reader(void *pvParameters) {
    char c;
    int strcur = 0;
    char tmpstr[10];
    Serial.flush();
    while (true) {
        while (Serial.available() && ((c = Serial.read()) == '#' || strcur != 0)) {
        tmpstr[strcur++] = c;
        if (strcur == 8) {
            tmpstr[strcur] = '\0';
            Serial.println(tmpstr);
            xQueueSend(instructions, tmpstr, 100); // 将指令发送到队列
            strcur = 0;
        }
        }

        if (!digitalRead(BUTTOM_START_PIN)) System_Start(); // 检查开始按钮是否按下
        if (!digitalRead(BUTTOM_RELAX_PIN)) System_Relax(); // 检查放松按钮是否按下
        if (!digitalRead(BUTTOM_RESET_PIN)) System_Reset(); // 检查重置按钮

        // if (!digitalRead(BtnRed)) Emergency_Stop();  // 检查紧急停止按钮
        // if (!digitalRead(BtnPrepare)) Motor_Prepare(); // 检查准备按钮
        
        vTaskDelay(5);
    }
}

void Instruction_Executant(void *pvParameters) {
    char ins[8];
    int device_id, operation;
    while (true) {
        if (xQueueIsQueueEmptyFromISR(instructions) == pdFALSE) {
        xQueueReceive(instructions, ins, 100);
        if (robot.isReady) {
            device_id = ins[1] - '0';  // 获取设备号
            operation = ins[3] - '0'; // 获取操作号

            switch (device_id) {
                case 1: case 3: 
                    if(device_id == 1) Serial.print("L");
                    else Serial.print("R");
                    Serial.println(operation);
                    if (robot.l.isTight && robot.r.isTight && operation >= 5) robot.curTwist = true;
                    if (robot.preTwist && robot.curTwist) delay(Continous_Twist_Delay); // 连续拧动延迟
                    Stepper_Control(device_id, operation); // 控制电机转动
                break;
                case 2: case 4: 
                    Hand_Control(device_id, operation); // 控制电磁铁开合
                break;
                case 0: case 7:
                    Serial.println("Ove");
                break;
                default: break;

            }
            robot.preTwist = robot.curTwist;
        }
        } else delay(100);
    }
}

// -------- 按钮相关函数 --------

// 系统放松
void System_Relax()
{
    delay(10);
    if (!digitalRead(BUTTOM_RELAX_PIN))
    {
        STEPPER_ALL_OFF();
        HAND_ALL_LOOSE();
        robot.Init();
        Serial.println("System_Relax");
        while (true)
        {
            if (digitalRead(BUTTOM_RELAX_PIN))
                break;
        }
    }
}

// 系统启动
void System_Start()
{
    delay(10);
    if (!digitalRead(BUTTOM_START_PIN))
    {
        Serial.println("Sta");
        while (true)
        {
            if (digitalRead(BUTTOM_START_PIN))
                break;
        }
    }
}

// 系统复原
void System_Reset() {
    delay(10);
    if (!digitalRead(BUTTOM_RESET_PIN)) {
        Serial.println("System_Reset");
        HAND_ALL_LOOSE();
        if (robot.l.degree % 180 != 0) {
            digitalWrite(STEPPER_L_DIR, (robot.l.degree > 0 ? LOW : HIGH));
            Pulse_Sender(STEPPER_L_PUL, round(PULSE360 * abs(robot.l.degree) / 360.0));
            delay(50);
            digitalWrite(STEPPER_R_DIR, (robot.r.degree > 0 ? LOW : HIGH));
            Pulse_Sender(STEPPER_R_PUL, round(PULSE360 * abs(robot.r.degree) / 360.0));
        }
        else {
            digitalWrite(STEPPER_R_DIR, (robot.r.degree > 0 ? LOW : HIGH));
            Pulse_Sender(STEPPER_R_PUL, round(PULSE360 * abs(robot.r.degree) / 360.0));
            delay(50);
            digitalWrite(STEPPER_L_DIR, (robot.l.degree > 0 ? LOW : HIGH));
            Pulse_Sender(STEPPER_L_PUL, round(PULSE360 * abs(robot.l.degree) / 360.0));
        }
        robot.Init();
        Stepper_Position_Init();

        while (true) {
            if (digitalRead(BUTTOM_RESET_PIN))
                break;
        }
    }
    return;
}

/*
void Motor_Prepare() {
  delay(10);
  if(!digitalRead(BtnPrepare)){
    //Serial.println("Motor_Prepare");
    digitalWrite(STEPPER_L_ENA, LOW);
    digitalWrite(STEPPER_R_ENA, LOW);
    delay(2000);
    Stepper_Position_Init();
    status=true;
    while(true){
      if(digitalRead(BtnPrepare))break;
    }
  }
  return;
}
*/


// 初始化各个引脚
void Pin_Mode_Init() {
    // 初始化电机引脚及加速度参数
	pinMode(STEPPER_L_PUL, OUTPUT);
	pinMode(STEPPER_L_DIR, OUTPUT);
	pinMode(STEPPER_L_ENA, OUTPUT);
	pinMode(STEPPER_L_ALM, INPUT_PULLUP);
	pinMode(STEPPER_R_PUL, OUTPUT);
	pinMode(STEPPER_R_DIR, OUTPUT);
	pinMode(STEPPER_R_ENA, OUTPUT);
	pinMode(STEPPER_R_ALM, INPUT_PULLUP);

    // 初始化电磁铁引脚
	pinMode(MAGNET_L_PIN, OUTPUT);
	pinMode(MAGNET_R_PIN, OUTPUT);

    // 初始化按钮引脚
    pinMode(BUTTOM_START_PIN, INPUT_PULLUP);
    pinMode(BUTTOM_RESET_PIN, INPUT_PULLUP);
	pinMode(BUTTOM_RELAX_PIN, INPUT_PULLUP);
    pinMode(BUTTOM_TIGHT_PIN, INPUT_PULLUP);
	pinMode(BUTTOM_LOOSE_PIN, INPUT_PULLUP);

    // 初始化霍尔传感器引脚
	pinMode(SENSOR_L_PIN, INPUT_PULLDOWN);
	pinMode(SENSOR_R_PIN, INPUT_PULLDOWN);
	pinMode(BUZZER_PIN, OUTPUT);
}