#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"


#define dbg_printf sDBG_Printf
#define dbg_info   sDBG_InfoPrintf
#define dbg_warn   sDBG_WarnPrintf

void sDBG_Printf(const char *fmt,...);
void sDBG_InfoPrintf(const char *fmt,...);
void sDBG_WarnPrintf(const char *fmt,...);

void sBSP_UART_Debug_Printf(const char *fmt,...);

void Error_Handler();
void Warning_Handler(uint8_t* file, uint32_t line);
void assert_failed(uint8_t* file, uint32_t line);

#ifdef __cplusplus
}
#endif

