# sGCARCv5

**Sightseer's General Car Controller v5**

通用小车控制器 v5

这是一个通用的轮式智能车控制器

开发环境:AC6+EIDE+HAL库

基于STM32F405RGT6:

SRAM :128+64K  FLASH :1024K  性能:168MHz 210DMIPS

## 他能用来干什么?

得益于v5.1的硬件设计,他能够用来驱动两个轮子的平衡车,三个轮子的万向轮小车,四轮阿克曼结构的舵机转向小车.





## 软件工程结构

sAPP应用层:包含上层的控制逻辑代码以及C++封装

sUSER用户杂项:

| 文件名                   | 用途                     |
| ------------------------ | ------------------------ |
| main.cpp/.h              | main                     |
| sGCARC_Def.h             | sGCARC硬件定义和引脚定义 |
| sGCARC_hal_msp.c/.h      | sGCARC硬件MSP初始化定义  |
| startup_stm32f405xx.s    | 汇编启动文件             |
| stm32f4xx_hal_conf.h     | HAL库配置文件            |
| stm32f4xx_hal_timebase.c | 使用TIM6作为滴答定时器   |
| stm32f4xx_it.c/.h        | 中断回调                 |
| stm32_assert.h           | 断言(未使用此文件)       |
| system_stm32f4xx.c/.h    | 系统文件                 |

sDBG调试配置:用于调试

sDRV驱动层:所有的硬件驱动文件都在这里

sLIB库文件:存放HAL库

sMID中间件:所有的库文件都在这里,比如cm_backtrace,FreeRTOS等

sBSP板级支持包:所有的硬件差异在这里被隔离






