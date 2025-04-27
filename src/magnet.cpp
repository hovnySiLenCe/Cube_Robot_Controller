#include "magnet.h"

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <thread>

// 根据指令设置电磁铁状态
// id：设备 ID，2 表示左手，4 表示右手
// op：操作 ID，0 表示打开，1 表示紧闭，2 表示松开，3 表示轻微打开，4 表示轻微紧闭
// 6 表示全紧闭，7 表示全松开
void Hand_Control(int id, int op) {
    switch (op) {
        case 0: if(id==2) L_HAND_LOOSE(); else R_HAND_LOOSE(); break;
        case 1: if(id==2) L_HAND_TIGHT(); else R_HAND_TIGHT(); break;
        case 6: HAND_ALL_TIGHT(); break;
        case 7: HAND_ALL_LOOSE(); break;
        case 9: if(id==2) Start_L_Hand_Tight_Long(); else Start_R_Hand_Tight_Long(); break;
        default: break;
    }
}

// 定义定时器句柄
TimerHandle_t l_hand_timer = NULL;
TimerHandle_t r_hand_timer = NULL;

// 左手磁铁定时器回调函数
void L_Hand_Timer_Callback(TimerHandle_t xTimer) {
    static bool isHigh = false; // 记录当前状态

    if (!robot.l.isTight) {
        xTimerStop(xTimer, 0); // 停止定时器
        return;
    }

    // 切换磁铁状态
    if (isHigh) {
        digitalWrite(MAGNET_L_PIN, LOW);
        xTimerChangePeriod(xTimer, pdMS_TO_TICKS(Magnet_Loose_Ratio_Delay), 0);
    } else {
        digitalWrite(MAGNET_L_PIN, HIGH);
        xTimerChangePeriod(xTimer, pdMS_TO_TICKS(Magnet_Tight_Ratio_Delay), 0);
    }
    isHigh = !isHigh; // 切换状态
}

void Start_L_Hand_Tight_Long() {
    robot.l.isTight = true;

    // 如果定时器尚未创建，创建定时器
    if (l_hand_timer == NULL) {
        l_hand_timer = xTimerCreate("L_Hand_Timer", pdMS_TO_TICKS(Magnet_Tight_Ratio_Delay), pdTRUE, NULL, L_Hand_Timer_Callback);
    }

    // 启动定时器
    xTimerStart(l_hand_timer, 0);
}

void Stop_L_Hand_Tight_Long() {
    robot.l.isTight = false;

    // 停止定时器
    if (l_hand_timer != NULL) {
        xTimerStop(l_hand_timer, 0);
    }
}

// 右手磁铁定时器回调函数
void R_Hand_Timer_Callback(TimerHandle_t xTimer) {
    static bool isHigh = false;

    if (!robot.r.isTight) {
        xTimerStop(xTimer, 0);
        return;
    }

    if (isHigh) {
        digitalWrite(MAGNET_R_PIN, LOW);
        xTimerChangePeriod(xTimer, pdMS_TO_TICKS(Magnet_Loose_Ratio_Delay), 0);
    } else {
        digitalWrite(MAGNET_R_PIN, HIGH);
        xTimerChangePeriod(xTimer, pdMS_TO_TICKS(Magnet_Tight_Ratio_Delay), 0);
    }
    isHigh = !isHigh;
}

void Start_R_Hand_Tight_Long() {
    robot.r.isTight = true;

    if (r_hand_timer == NULL) {
        r_hand_timer = xTimerCreate("R_Hand_Timer", pdMS_TO_TICKS(Magnet_Tight_Ratio_Delay), pdTRUE, NULL, R_Hand_Timer_Callback);
    }

    xTimerStart(r_hand_timer, 0);
}

void Stop_R_Hand_Tight_Long() {
    robot.r.isTight = false;

    if (r_hand_timer != NULL) {
        xTimerStop(r_hand_timer, 0);
    }
}

void L_Hand_Tight_Long() {
    L_HAND_TIGHT();
    std::thread([]() {
        while(robot.l.isTight) {
            digitalWrite(MAGNET_L_PIN, HIGH);
            delay(Magnet_Tight_Ratio_Delay);
            digitalWrite(MAGNET_L_PIN, LOW);
            delay(Magnet_Loose_Ratio_Delay);
        }
    }).detach();
}

void R_Hand_Tight_Long() {
    R_HAND_TIGHT();
    std::thread([]() {
        while(robot.r.isTight) {
            digitalWrite(MAGNET_R_PIN, HIGH);
            delay(Magnet_Tight_Ratio_Delay);
            digitalWrite(MAGNET_R_PIN, LOW);
            delay(Magnet_Loose_Ratio_Delay);
        }
    }).detach();
}