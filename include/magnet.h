#ifndef _MAGNET_H_
#define _MAGNET_H_

#include "system.h"

#define L_HAND_TIGHT()      do { digitalWrite(MAGNET_L_PIN, HIGH);delay(Magnet_Delay);robot.l.isTight=true; } while(0)
#define R_HAND_TIGHT()      do { digitalWrite(MAGNET_R_PIN, HIGH);delay(Magnet_Delay);robot.r.isTight=true; } while(0)
#define HAND_ALL_TIGHT()    do { L_HAND_TIGHT();R_HAND_TIGHT(); } while(0)

#define L_HAND_LOOSE()      do { digitalWrite(MAGNET_L_PIN, LOW);delay(Magnet_Delay);robot.l.isTight=false; } while(0)
#define R_HAND_LOOSE()      do { digitalWrite(MAGNET_R_PIN, LOW);delay(Magnet_Delay);robot.r.isTight=false; } while(0)
#define HAND_ALL_LOOSE()    do { L_HAND_LOOSE();R_HAND_LOOSE(); } while(0)

void Hand_Control(int id, int op);

#endif