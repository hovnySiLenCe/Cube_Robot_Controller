#include "main.h"
#include <Preferences.h>

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
void Robot_Monitor_t:: HandConvert() {
    delay(100);
    if (!digitalRead(BUTTOM_HAND_PIN))
    {
        bool toTight = !(l.isTight | r.isTight);
        if(toTight) HAND_ALL_TIGHT();
        else HAND_ALL_LOOSE();
        l.isTight = r.isTight = toTight;
        while (true)
        {
            if (digitalRead(BUTTOM_START_PIN))
                break;
        }
    }
}

Robot_Monitor_t robot;
void setup() {
    esp_log_level_set("*", ESP_LOG_NONE);
    esp_task_wdt_init(30, false);
    //esp_log_level_set("esp_system", ESP_LOG_NONE);
    // 初始化引脚
	Pin_Mode_Init();
    HAND_ALL_LOOSE(); // 松开所有电磁铁
    STEPPER_ALL_ON(); // 使能电机
    // 初始化蜂鸣器
    //digitalWrite(BUZZER_PIN, LOW);

    Serial.begin(9600);
    Serial.println("");
    robot.Init();
    Data_Sheet_Init();
    Stepper_Acc_Init();
    
	instructions = xQueueCreate(200, 9 * sizeof(char)); // 创建队列
	if (instructions == NULL) {
		Serial.println("[FATAL] failed to create queue");
	}
	xTaskCreatePinnedToCore(Serial_Reader, "Serial_Reader", 10000, NULL, 3, &reader, 0);
	delay(500);
	xTaskCreatePinnedToCore(Instruction_Executant, "Instruction_Executant", 10000, NULL, 3, &executant, 1);
    delay(500);

    Stepper_Position_Init();
	Serial.println("---------- Successfully Initialized ----------");
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
            if (c == '\n' || c== '\r' || strcur == 8) {
                tmpstr[strcur] = '\0';
                //Serial.printf("received: %s\n", tmpstr);
                switch (tmpstr[1]) {
                case '8':
                    System_Relax(true); // 系统放松
                    break;
                case '9':
                    System_Reset(true); // 系统复原
                    break;
                default:
                    xQueueSend(instructions, tmpstr, 100);
                    break;
                }
                strcur = 0;
            }
        }

        if (!digitalRead(BUTTOM_START_PIN)) System_Start(); // 检查开始按钮是否按下
        if (!digitalRead(BUTTOM_RELAX_PIN)) System_Relax(false); // 检查放松按钮是否按下
        if (!digitalRead(BUTTOM_RESET_PIN)) System_Reset(false); // 检查重置按钮
        if (!digitalRead(BUTTOM_HAND_PIN)) robot.HandConvert(); // 检查手爪开合按钮

        // if (!digitalRead(BtnRed)) Emergency_Stop();  // 检查紧急停止按钮
        // if (!digitalRead(BtnPrepare)) Motor_Prepare(); // 检查准备按钮
        
        vTaskDelay(5);
    }
}

int String2Int(char *str) {
    int result = 0;
    while(*str && *str != '\n' && *str != '\r') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

void Instruction_Executant(void *pvParameters) {
    char ins[8], device_id, operation;
    int value;
    while (true) {
        if (xQueueIsQueueEmptyFromISR(instructions) == pdFALSE) {
        xQueueReceive(instructions, ins, 100);
        if (robot.isReady) {
            device_id = ins[1];  // 获取设备号
            operation = ins[3]; // 获取操作号
            
            if(device_id != 'W') Serial.print("[INFO] Operated: ");

            if(device_id == '1' || device_id == '2') Serial.print("L");
            else if(device_id == '3' || device_id == '4') Serial.print("R");

            switch (device_id) {
                case '1': case '3': 
                    //Serial.println(operation);
                    if (robot.l.isTight && robot.r.isTight && operation >= '5') robot.curTwist = true;
                    if (robot.preTwist && robot.curTwist) delay(Continous_Twist_Delay); // 连续拧动延迟
                    Stepper_Control(device_id-'0', operation-'0'); // 控制电机转动
                break;
                case '2': case '4':
                    Serial.println((operation-'0')?"Close":"Open");
                    Hand_Control(device_id-'0', operation-'0'); // 控制电磁铁开合
                break;
                case '0': case '7':
                    Serial.println("#Over");
                break;
                case 'R':
                    Serial.println("Date_Sheet_Read");
                    Data_Sheet_Read();
                break;
                case 'W':
                    //Serial.printf("Modify: %d\n", String2Int(ins + 2));
                    Data_Sheet_Modify(String2Int(ins + 2));
                break;
                case 'E':
                    Serial.println("Date_Sheet_Save");
                    Data_Sheet_Save();
                break;
                default: break;
            }
            robot.preTwist = robot.curTwist;
        }
        } else delay(100);
    }
}

// -------- 按钮相关函数 --------

// 系统启动
void System_Start()
{
    delay(10);
    if (!digitalRead(BUTTOM_START_PIN))
    {
        Serial.println("#Start");
        while (true)
        {
            if (digitalRead(BUTTOM_START_PIN))
                break;
        }
    }
}

// 系统放松
void System_Relax(bool isFromPC)
{
    delay(10);
    if (!digitalRead(BUTTOM_RELAX_PIN) || isFromPC)
    {
        STEPPER_ALL_OFF();
        HAND_ALL_LOOSE();
        robot.isReady = false;
        Serial.println("#Relax");
        while (true)
        {
            if (digitalRead(BUTTOM_RELAX_PIN) || isFromPC)
                break;
        }
    }
}

// 系统复原
void System_Reset(bool isFromPC) {
    delay(10);
    if (!digitalRead(BUTTOM_RESET_PIN) || isFromPC) {
        Serial.println("#Reset");
        HAND_ALL_LOOSE();
        STEPPER_ALL_ON();
        vTaskDelay(100);
        robot.isDebug = true;
        if (robot.l.degree % 180 != 0) {
            digitalWrite(STEPPER_L_DIR, (robot.l.degree > 0 ? ACW : CW));
            Pulse_Sender(STEPPER_L_PUL, round(PULSE360 * abs(robot.l.degree) / 360.0));
            vTaskDelay(500);
            digitalWrite(STEPPER_R_DIR, (robot.r.degree > 0 ? ACW : CW));
            Pulse_Sender(STEPPER_R_PUL, round(PULSE360 * abs(robot.r.degree) / 360.0));
        }
        else {
            digitalWrite(STEPPER_R_DIR, (robot.r.degree > 0 ? ACW : CW));
            Pulse_Sender(STEPPER_R_PUL, round(PULSE360 * abs(robot.r.degree) / 360.0));
            vTaskDelay(500);
            digitalWrite(STEPPER_L_DIR, (robot.l.degree > 0 ? ACW : CW));
            Pulse_Sender(STEPPER_L_PUL, round(PULSE360 * abs(robot.l.degree) / 360.0));
        }

        vTaskDelay(5);

        robot.Init();
        Stepper_Position_Init();

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
    pinMode(BUTTOM_HAND_PIN, INPUT_PULLUP);

    // 初始化霍尔传感器引脚
	pinMode(SENSOR_L_PIN, INPUT_PULLDOWN);
	pinMode(SENSOR_R_PIN, INPUT_PULLDOWN);
	pinMode(BUZZER_PIN, OUTPUT);
}