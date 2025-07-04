#pragma once


#include "main.h"
#include "sDRV_YaHaboom_voice.h"


void sAPP_Tasks_CreateAll();


void sAPP_Tasks_PrintTaskMang();

void     sAPP_Tasks_UltrasonicTask(void* param);
uint32_t sAPP_Tasks_GetUltrasonicDistance(void);
bool     sAPP_Tasks_IsUltrasonicDataValid(void);


void sAPP_Tasks_AHRS(void* param);


void sAPP_Tasks_StartCalibAccBias();
void sAPP_Tasks_StartCalibGyrBias();
void sAPP_Tasks_ReadIMUCaliVal();


void sAPP_Tasks_StartFormatFeRAM();
void sAPP_Tasks_StartCalibrateIMU();




void           sAPP_Tasks_VoiceRecognition(void* param);
voice_result_t sAPP_Tasks_GetVoiceResult(void);
bool           sAPP_Tasks_IsVoiceDataValid(void);
