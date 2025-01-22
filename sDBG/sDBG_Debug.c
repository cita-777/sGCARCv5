#include "sDBG_Debug.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "sBSP_UART.h"

//此文件用于调试

char fmt_buf[512];



void sDBG_Printf(const char *fmt,...){
    va_list args;
    va_start(args, fmt);
    vsprintf(fmt_buf, fmt, args);
    va_end(args);
    //串口1输出
    sBSP_UART_Debug_Printf("%s",fmt_buf);
}

void sDBG_InfoPrintf(const char *fmt,...){
    va_list args;
    va_start(args, fmt);
    vsprintf(fmt_buf, fmt, args);
    va_end(args);
    //串口1输出
    sBSP_UART_Debug_Printf("[INFO] %s",fmt_buf);
}

void sDBG_WarnPrintf(const char *fmt,...){
    va_list args;
    va_start(args, fmt);
    vsprintf(fmt_buf, fmt, args);
    va_end(args);
    //串口1输出
    sBSP_UART_Debug_Printf("[WARN] %s",fmt_buf);
}



void Error_Handler(){
    __disable_irq();
    while (1){
        sBSP_UART_Debug_Printf("[ERROR] 错误: 禁用IRQ,死循环...\n");
        HAL_Delay(500);
    }
}

void Warning_Handler(uint8_t* file, uint32_t line){
    sBSP_UART_Debug_Printf("WARNING警告: 文件名:%s,行:%u\n",file,line);
}




void assert_failed(uint8_t* file, uint32_t line){
    __disable_irq();
    while (1){
        sBSP_UART_Debug_Printf("WARNING警告:断言失败! 文件名:%s,行:%u 禁用IRQ,死循环...\n",file,line);
        HAL_Delay(500);
    }
}

void vApplicationMallocFailedHook(){
    sBSP_UART_Debug_Printf("WARNING警告:内存申请失败! 禁用IRQ\n");
}

void vApplicationIdleHook(){
    
}   

void vApplicationTickHook(){
}


void vApplicationStackOverflowHook(TaskHandle_t xTask,char* pcTaskName){
    sBSP_UART_Debug_Printf("WARNING警告:触发栈金丝雀机制! 任务句柄:0x%X,任务名:%s\n",xTask,pcTaskName);
}


