#pragma once
#ifdef __cplusplus
extern "C"{
#endif

#include "stm32f4xx_hal.h"


void sBSP_RNG_Init();

uint32_t sBSP_RNG_GetU32();
uint16_t sBSP_RNG_GetU16();
uint8_t sBSP_RNG_GetU8();

float sBSP_RNG_GetRangeFloat(float min,float max);



#ifdef __cplusplus
}
#endif
