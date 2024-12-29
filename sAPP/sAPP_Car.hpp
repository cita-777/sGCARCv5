#pragma once


#include "stm32f4xx_hal.h"


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"



class sAPP_Car{
public:
    sAPP_Car();
    ~sAPP_Car();

    SemaphoreHandle_t mutex;

    uint32_t coreClock;
    float mcu_temp;
    float mcu_volt;

    float batt_volt;
    float batt_curr;
    float batt_power;
    


    int initSys();
    int initBoard();
    


private:


};


extern"C" void sAPP_Car_InfoUpdateTask(void* param);


extern sAPP_Car car;



