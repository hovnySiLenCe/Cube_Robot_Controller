#include "magnet.h"


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
        default: break;
    }
}
