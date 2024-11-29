#pragma once
#ifdef __cplusplus
extern "C"{
#endif

#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "stm32f4xx_hal.h"

#include "sBSP_UART.h"


//模块回传数据
typedef struct{
    float acc_x, acc_y, acc_z;
    float gyr_x, gyr_y, gyr_z;
    float pitch, roll , yaw;
    float mag_x, mag_y, mag_z;
    float q0, q1, q2, q3;
    float temp;
}sDRV_JY901S_t;

extern sDRV_JY901S_t g_jy901s;







int sDRV_JY901S_Init();

void sDRV_JY901S_SetBandrate115200();
void sDRV_JY901S_SetRRATENever();
void sDRV_JY901S_SetRRATE10Hz();
void sDRV_JY901S_SetRRATE100Hz();
void sDRV_JY901S_SetDegRef();


void sDRV_JY901S_RecvDataCb(char* pReciData,uint16_t length);


void sDRV_JY901S_Handler();



#ifdef __cplusplus
}
#endif
