#include "sAPP_Tasks.hpp"

#include "main.h"


void sAPP_Tasks_AHRS(void* param){
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    for(;;){
        ahrs.update();
        motor.update();
        sAPP_BlcCtrl_Handler();

        // sBSP_UART_Debug_Printf("pitch: %6.2f, roll: %6.2f, yaw: %6.2f\n",ahrs.pitch,ahrs.roll,ahrs.yaw);

        //高精确度延时10ms
        xTaskDelayUntil(&xLastWakeTime,10 / portTICK_PERIOD_MS);
    }
}

void sAPP_Tasks_OLEDHdr(void* param){
    for(;;){
        oled.printf(10,20,"%6.2f deg",ahrs.pitch);
        oled.printf(10,30,"%6.2f deg",ahrs.roll);
        oled.printf(10,40,"%6.2f deg",ahrs.yaw);

        oled.handler();
        oled.setAll(0);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


void sAPP_Tasks_20ms(void* param){
    for(;;){
        //处理按键
        sGBD_Handler();
        //处理二值化设备
        BinOutDrv.update();

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}


void sAPP_Tasks_500ms(void* param){

    for(;;){
        // led.toggle();

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}



void sAPP_Tasks_CreateAll(){
    //姿态估计算法 100Hz
    //xTaskCreate(sAPP_Tasks_AHRS         , "AHRS"       , 2048 / sizeof(int), NULL, 2, NULL);
    //OLED刷屏 20Hz
    xTaskCreate(sAPP_Tasks_OLEDHdr      , "OLED"       , 2048 / sizeof(int), NULL, 1, NULL);
    xTaskCreate(sAPP_Tasks_20ms         , "20ms"       ,  512 / sizeof(int), NULL, 1, NULL);
    xTaskCreate(sAPP_Tasks_500ms        , "500ms"      ,  512 / sizeof(int), NULL, 1, NULL);
    
}








