#pragma once
#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "sGenBtnDrv2.h"
#include "sBinOutDev.hpp"
#include "sAPP_BinOutDev.hpp"


#define SGBD_KEY_UP_ID       0
#define SGBD_KEY_DN_ID       1
#define SGBD_KEY_ET_ID       2
#define SGBD_KEY_BK_ID       3

#define SGBD_KEY_PS2_UP_ID   4
#define SGBD_KEY_PS2_DN_ID   5
#define SGBD_KEY_PS2_LE_ID   6
#define SGBD_KEY_PS2_RI_ID   7
#define SGBD_KEY_PS2_L2_ID   8
#define SGBD_KEY_PS2_L1_ID   9


void sAPP_Btns_Init();

#ifdef __cplusplus
}
#endif
