#pragma once


#include "main.h"


void sAPP_Tasks_CreateAll();


void sAPP_Tasks_PrintTaskMang();




void sAPP_Tasks_AHRS(void* param);


void sAPP_Tasks_StartCalibAccBias();
void sAPP_Tasks_StartCalibGyrBias();
void sAPP_Tasks_ReadIMUCaliVal();


void sAPP_Tasks_StartFormatFeRAM();
void sAPP_Tasks_StartCalibrateIMU();

