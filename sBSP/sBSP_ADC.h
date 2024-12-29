#pragma once
#ifdef __cplusplus
extern "C" {
#endif



#include "stm32f4xx_hal.h"




void sBSP_ADC_Init();


float sBSP_ADC_GetMCUTemp();
float sBSP_ADC_GetVCC();


#ifdef __cplusplus
}
#endif



