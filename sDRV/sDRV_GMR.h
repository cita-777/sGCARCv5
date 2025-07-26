#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"


void sDRV_GMR_Init();

float sDRV_GMR_GetLeftRPM();
float sDRV_GMR_GetRightRPM();
void  sDRV_GMR_Handler();

// 距离查询API
float sDRV_GMR_GetLeftDistance();
float sDRV_GMR_GetRightDistance();
void  sDRV_GMR_ResetDistance();



#ifdef __cplusplus
}
#endif
