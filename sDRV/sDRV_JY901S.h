#pragma once
#ifdef __cplusplus
extern "C"{
#endif

#include "stdbool.h"
#include "stdint.h"
#include "stm32f4xx_hal.h"

#include "sBSP_UART.h"


//SAVE 保存/重启/恢复出厂 RW ADDR:0x00
#define JY901S_SAVE_SAVE           "\x00"
#define JY901S_SAVE_REBOOT         "\xFF"
#define JY901S_SAVE_RESTORE        "\x01"

//CALSW 校准模式 RW ADDR:0x01 
#define JY901S_CALSW_NORMAL        "\x00" //正常工作模式
#define JY901S_CALSW_AUTOCAL       "\x01" //自动加计校准
#define JY901S_CALSW_ALTCLR        "\x03" //高度清零
#define JY901S_CALSW_HEADINGCLR    "\x04" //航向角置0
#define JY901S_CALSW_MAGCAL1       "\x07" //磁场校准(球形拟合法)
#define JY901S_CALSW_SETDEGREF     "\x08" //设置角度参考
#define JY901S_CALSW_MAGCAL2       "\x09" //磁场校准(双平面模式)

//RSW 输出内容 RW ADDR:0x02
#define JY901S_RSW_GSA             "\x04\x00" //0x01 << 10 = 0x0400
#define JY901S_RSW_QUATER          "\x02\x00" //0x01 << 9  = 0x0200
#define JY901S_RSW_VELOCITY        "\x01\x00" //0x01 << 8  = 0x0100
#define JY901S_RSW_GPS             "\x00\x80" //0x01 << 7  = 0x0080
#define JY901S_RSW_PRESS           "\x00\x40" //0x01 << 6  = 0x0040
#define JY901S_RSW_PORT            "\x00\x20" //0x01 << 5  = 0x0020
#define JY901S_RSW_MAG             "\x00\x10" //0x01 << 4  = 0x0010
#define JY901S_RSW_ANGLE           "\x00\x08" //0x01 << 3  = 0x0008
#define JY901S_RSW_GYRO            "\x00\x04" //0x01 << 2  = 0x0004
#define JY901S_RSW_ACC             "\x00\x02" //0x01 << 1  = 0x0002
#define JY901S_RSW_TIME            "\x00\x01" //0x01 << 0  = 0x0001


//RRATE 输出速率寄存器 ADDR:0x03
#define JY901S_RRATE_0D2HZ         "\x01"
#define JY901S_RRATE_0D5HZ         "\x02"
#define JY901S_RRATE_1HZ           "\x03"
#define JY901S_RRATE_2HZ           "\x04"
#define JY901S_RRATE_5HZ           "\x05"
#define JY901S_RRATE_10HZ          "\x06"
#define JY901S_RRATE_20HZ          "\x07"
#define JY901S_RRATE_50HZ          "\x08"
#define JY901S_RRATE_100HZ         "\x09"
#define JY901S_RRATE_200HZ         "\x0B"
#define JY901S_RRATE_ONCE          "\x0C" //单次回传
#define JY901S_RRATE_NEVER         "\x0D" //不回传

//BANDWIDTH 带宽 RW ADDR:0x1F 默认20Hz
#define JY901S_BANDWIDTH_256HZ     "\x00"
#define JY901S_BANDWIDTH_188HZ     "\x01"
#define JY901S_BANDWIDTH_98HZ      "\x02"
#define JY901S_BANDWIDTH_42HZ      "\x03"
#define JY901S_BANDWIDTH_20HZ      "\x04"
#define JY901S_BANDWIDTH_10HZ      "\x05"
#define JY901S_BANDWIDTH_5HZ       "\x06"

//AXIS6 算法 RW ADDR:0x24
#define JY901S_AXIS6_9AXIS         "\x00" //9轴算法 绝对航向角
#define JY901S_AXIS6_6AXIS         "\x01" //6轴算法 相对航向角







int sDRV_JY901S_Init();

void sDRV_JY901S_SetBandrate115200();


void sDRV_JY901S_Handler();



#ifdef __cplusplus
}
#endif
