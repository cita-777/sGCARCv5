#include "sAPP_BinOutDev.hpp"



void sAPP_BOD_Init(){
    BinOutDrv.init();
    BinOutDrv.addDev(BOD_LED_ID,GPIOC,GPIO_PIN_13);
    BinOutDrv.addDev(BOD_BUZZER_ID,GPIOB,GPIO_PIN_15);
    
    BinOutDrv.confDevMode(BOD_LED_ID,sBOD::DEV_MODE::ASYMMETRIC_TOGGLE,sBOD::LEVEL::LOW);
    BinOutDrv.confDevMode(BOD_BUZZER_ID,sBOD::DEV_MODE::PULSE_HIGH,sBOD::LEVEL::LOW);
    BinOutDrv.confTime(BOD_LED_ID,1000,100);
    BinOutDrv.confTime(BOD_BUZZER_ID,100,50);
}


