
#ifndef SDRV_YAHABOOM_VOICE_H
#define SDRV_YAHABOOM_VOICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------include-----------------------------------*/
#include "sBSP_I2C.h"
#include "stm32f4xx_hal.h"

/*-----------------------------------macro------------------------------------*/
#define VOICE_ADDR 0x2B       // 语音模块I2C地址
#define WRITE_REGISTER 0x03   // 写寄存器命令
#define READ_REGISTER 0x64    // 读寄存器命令

// 主动播报内容定义
#define VOICE_THIS_RED 0x66           // 播报"检测到红色"
#define VOICE_THIS_BLUE 0x65          // 播报"检测到蓝色"
#define VOICE_THIS_GREEN 0x64         // 播报"检测到绿色"
#define VOICE_THIS_YELLOW 0x63        // 播报"检测到黄色"
#define VOICE_RECOGNIZE_YELLOW 0x62   // 识别到黄色
#define VOICE_RECOGNIZE_GREEN 0x61    // 识别到绿色
#define VOICE_RECOGNIZE_BLUE 0x60     // 识别到蓝色
#define VOICE_RECOGNIZE_RED 0x5F      // 识别到红色
#define VOICE_INIT 0x67               // 初始化语音

/*----------------------------------typedef-----------------------------------*/
// 语音识别结果枚举
typedef enum
{
    VOICE_RESULT_NONE = 0,
    VOICE_RESULT_RED,
    VOICE_RESULT_BLUE,
    VOICE_RESULT_GREEN,
    VOICE_RESULT_YELLOW,
    VOICE_RESULT_INIT
} voice_result_t;

/*----------------------------------variable----------------------------------*/

/*-------------------------------------os-------------------------------------*/

/*----------------------------------function----------------------------------*/
// 语音模块初始化
int8_t sDRV_YaHaboom_Voice_Init(void);

// 播报指定内容
int8_t sDRV_YaHaboom_Voice_Broadcast(uint8_t data);

// 读取语音识别数据
voice_result_t sDRV_YaHaboom_Voice_ReadData(void);

// 语音识别任务处理函数
void sDRV_YaHaboom_Voice_Handler(void);

/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* SDRV_YAHABOOM_VOICE_H */
