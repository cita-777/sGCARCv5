#pragma once


#include "main.h"

#include "sAPP_AHRS.hpp"


#include "sGCARC_Def.h"



#include "sDRV_MB85RCxx.h"




void sAPP_ParamSave_Init();


int sAPP_ParamSave_SaveGyrSBias(float x_bias,float y_bias,float z_bias,float temp);
int sAPP_ParamSave_ReadGyrSBias(float* x_bias,float* y_bias,float* z_bias,float* temp);
int sAPP_ParamSave_SaveAccSBias(float x_bias,float y_bias,float z_bias,float temp);
int sAPP_ParamSave_ReadAccSBias(float* x_bias,float* y_bias,float* z_bias,float* temp);

void sAPP_ParamSave_CaliIMU();










