#pragma once


#include "main.h"
// 删除了语音相关的头文件包含


void sAPP_Tasks_CreateAll();


void sAPP_Tasks_PrintTaskMang();

// 差速两轮小车电机控制任务
void sAPP_Tasks_MotorControlTask(void* param);

// 步进电机测试任务
void sAPP_Tasks_StepperMotorTest(void* param);

// IMU校准相关函数 - 保留
void sAPP_Tasks_StartCalibAccBias();
void sAPP_Tasks_StartCalibGyrBias();
void sAPP_Tasks_ReadIMUCaliVal();

// FeRAM和IMU校准相关 - 保留
void sAPP_Tasks_StartFormatFeRAM();
void sAPP_Tasks_StartCalibrateIMU();

// 灰度传感器测试任务
void sAPP_Tasks_GrayScaleTest(void* param);

// UART6主机通信测试任务
void sAPP_Tasks_UART6_HostCommTest(void* param);

// UART6主机通信测试任务
void sAPP_Tasks_UART6_HostCommTest(void* param);
