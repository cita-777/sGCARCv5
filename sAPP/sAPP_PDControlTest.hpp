#pragma once

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

// PD控制系统测试任务
void sAPP_Tasks_PDControlTest(void* param);

// 简化的PD控制演示任务
void sAPP_Tasks_SimplePDDemo(void* param);
