#include "sBSP_GPIO.h"

#include "sDRV_ICM45686.h"

#include "sBSP_UART.h"

#include "sGCARC_Def.h"





void sBSP_GPIO_IcmInt_Init(){
    GPIO_InitTypeDef gpio = {0};
    gpio.Mode  = GPIO_MODE_IT_FALLING;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_MEDIUM;
    gpio.Pin   = IMU_INT1_Pin;
    HAL_GPIO_Init(IMU_INT1_GPIO_Port,&gpio);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}


void ISR_ATTR HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    if(GPIO_Pin == IMU_INT1_Pin){
        //通知AHRS,ICM的数据准备好了
        extern void sAPP_AHRS_ICMDataReadyCbISR();
        sAPP_AHRS_ICMDataReadyCbISR();
    }
}







