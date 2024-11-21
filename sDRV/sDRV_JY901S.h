#pragma once
#ifdef __cplusplus
extern "C"{
#endif

#include "stdbool.h"
#include "stdint.h"
#include "stm32f4xx_hal.h"

#include "sBSP_UART.h"

















int sDRV_JY901S_Init();

void sDRV_JY901S_SetBandrate115200();
void sDRV_JY901S_SetRRATENever();
void sDRV_JY901S_SetRRATE10Hz();




void sDRV_JY901S_Handler(char* pReciData,uint16_t length);



#ifdef __cplusplus
}
#endif
