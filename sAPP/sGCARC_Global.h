#pragma once


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"




// QueueHandle_t g_btn;

//IMU数据就绪二值信号量
extern SemaphoreHandle_t g_ahrs_imu_drdy;





