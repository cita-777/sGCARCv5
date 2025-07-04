#pragma once


#include "stm32f4xx_hal.h"

#include "sGCARC_Def.h"
#include "sGCARC_hal_msp.h"

#include "arm_math.h"
#include "math.h"

#include "sDBG_Debug.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"


#include "sBinOutDev.hpp"
#include "sG2D.hpp"


// #include <string>




/*APP*/
#include "sAPP_AHRS.hpp"
#include "sAPP_BinOutDev.hpp"
#include "sAPP_BlcCtrl.hpp"
#include "sAPP_Btns.hpp"
#include "sAPP_Car.hpp"
#include "sAPP_GUI.hpp"
#include "sAPP_Motor.hpp"
#include "sAPP_ParamSave.hpp"
#include "sAPP_Tasks.hpp"

#include "ekf_AltEst6.h"




/*MID*/
// #include "sGraphic2D.h"
#include "cm_backtrace.h"
#include "sDRV_GenOLED.h"

#include "sDWTLib.hpp"

#include "sLM_OLED128X64.hpp"
#include "sLittleMenu.hpp"
#include "sightseerUtils\sUtils.h"



/*LIB*/
#include "sLib.h"

/*DRV*/
#include "sDRV_DRV8870.h"
#include "sDRV_GMR.h"
#include "sDRV_HC_SR04.h"
#include "sDRV_ICM45686.h"
#include "sDRV_INA219.hpp"

#include "sDRV_LIS3MDLTR.h"
#include "sDRV_MB85RCxx.h"
#include "sDRV_PS2.h"
#include "sDRV_PwrLight.h"
#include "sDRV_TrackTube8.h"
#include "sDRV_YaHaboom_voice.h"

/*BSP*/
#include "sBSP_ADC.h"
#include "sBSP_DMA.h"
#include "sBSP_I2C.h"
#include "sBSP_RCC.h"
#include "sBSP_RNG.h"
#include "sBSP_SPI.h"
#include "sBSP_SYS.h"
#include "sBSP_TIM.h"
#include "sBSP_UART.h"




#define delay(__MS__) vTaskDelay(__MS__)


void loop();
