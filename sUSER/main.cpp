#include "main.h"





void uart_recied(char* pReciData,uint16_t length){
    sBSP_UART_Debug_SendBytes((uint8_t*)pReciData,length);
    //sBSP_UART_Debug_Printf("%s\n",pReciData);
    //parseSerialData(pReciData);

    sBSP_UART_IMU_RecvBegin(uart_recied);
}




sBinOutDrv bod;

void output(GPIO_TypeDef* group,uint16_t pin,GPIO_PinState lv){
    HAL_GPIO_WritePin(group,pin,(GPIO_PinState)lv);
}


int main(){
    car.initSys();
    car.initBoard();


    //sBSP_UART_Debug_RecvBegin(uart_recied);
    //sBSP_UART_Top_RecvBegin(uart_recied);

    sBSP_UART_Debug_Printf("STM32 System Clock Freq: %u MHz\n", car.coreClk / 1000000);
    sBSP_UART_Debug_Printf("Hello,STM32F405RGT6    BySightseer.\n");
    sBSP_UART_Debug_Printf("sGCARC初始化完成\n");

    sDRV_GenOLED_Init();
    oled.init();
    oled.setFPSMode(sG2D::DIGITS2);

    ahrs.init();
    

    int i = 0;

    sAPP_BlcCtrl_Init();

    sDRV_PS2_Init();
    sDRV_PL_Init();
    sDRV_PL_SetBrightness(50);


    // sBSP_DWT_MeasureStart();
    // sBSP_DWT_MeasureEnd();
    // sBSP_UART_Debug_Printf("%uus\n",sBSP_DWT_GetMeasure_us());

    #define LED_ID 0
    bod.init();
    bod.regOutputCb(output);
    bod.regGetTick(HAL_GetTick);
    bod.addDev(GPIOC,GPIO_PIN_13,LED_ID);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    bod.addDev(GPIOC,GPIO_PIN_13,5,false);
    
    bod.confDevMode(LED_ID,sBinOutDrv::dev_mode_t::SYMMETRIC_TOGGLE,false);
    bod.confTime(LED_ID,1000,100);

    sAPP_Tasks_CreateAll();
    sDBG_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
    sBSP_UART_Debug_Printf("FreeRTOS启动任务调度\n");
    //vTaskStartScheduler();    
    

    // while(1);
    while(1){
        sBSP_DWT_MeasureStart();
        bod.handler();
        sBSP_DWT_MeasureEnd();
        sBSP_UART_Debug_Printf("%uus\n",sBSP_DWT_GetMeasure_us());
        HAL_Delay(20);

        // oled.printf(10,50,"%u",i);
        // i++;
        // //sBSP_DWT_MeasureStart();
        // oled.handler();
        // oled.setAll(0);
        // sBSP_UART_Top_Printf("Hello,ros2,this is stm32f405,i=%u\n",i);

        // ahrs.update();


        //         // sBSP_DWT_MeasureEnd();
        // //sBSP_UART_Debug_Printf("%uus\n",sBSP_DWT_GetMeasure_us());

        // //sBSP_UART_Debug_Printf("q0:%.2f, q1:%.2f, q2:%.2f, q3:%.2f\n",\
        // (float)sensorData.data1, (float)sensorData.data2, (float)sensorData.data3, (float)sensorData.data4);
        


        // led.toggle();


        // HAL_Delay(50);
        

        

    }
}


void* operator new(std::size_t size) {
    return pvPortMalloc(size);  // FreeRTOS memory allocation
}

void operator delete(void* ptr) noexcept {
    vPortFree(ptr);  // FreeRTOS memory free
}

void* operator new[](std::size_t size) {
    return pvPortMalloc(size);  // For array allocation
}

void operator delete[](void* ptr) noexcept {
    vPortFree(ptr);  // For array deallocation
}





