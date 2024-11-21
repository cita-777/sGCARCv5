#include "main.h"



//128+64K SRAM
//1024K FLASH
//168MHz 210DMIPS

//compile env:
//AC6 C++14


void parseSerialData(const char* data);

void uart_recied(char* pReciData,uint16_t length){
    sBSP_UART_Debug_SendBytes((uint8_t*)pReciData,length);
    //sBSP_UART_Debug_Printf("%s\n",pReciData);
    //parseSerialData(pReciData);

    sBSP_UART_IMU_RecvBegin(uart_recied);
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

    
    sDRV_JY901S_Init();
    sBSP_UART_IMU_RecvBegin(sDRV_JY901S_Handler);
    //sBSP_UART_IMU_RecvBegin(uart_recied);
    HAL_Delay(3000);
    sDRV_JY901S_SetRRATE10Hz();




    int i = 0;

    size_t freeHeapSize = 0;
    freeHeapSize = xPortGetFreeHeapSize();
    sDBG_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)freeHeapSize);


    // sAPP_Tasks_CreateAll();
    // sBSP_UART_Debug_Printf("FreeRTOS启动任务调度\n");
    // vTaskStartScheduler();    
    

    //while(1);
    while(1){
        oled.printf(10,50,"%u",i);
        i++;
        //sBSP_DWT_MeasureStart();
        oled.handler();
        oled.setAll(0);
        sBSP_UART_Top_Printf("Hello,ros2,this is stm32f405,i=%u\n",i);
                // sBSP_DWT_MeasureEnd();
        //sBSP_UART_Debug_Printf("%uus\n",sBSP_DWT_GetMeasure_us());

        //sBSP_UART_Debug_Printf("q0:%.2f, q1:%.2f, q2:%.2f, q3:%.2f\n",\
        (float)sensorData.data1, (float)sensorData.data2, (float)sensorData.data3, (float)sensorData.data4);
        



        // sBSP_UART_Debug_Printf("TYPE: 0x%02X", sensorData.type);
        // sBSP_UART_Debug_Printf(" DATA1: %.2f", (float)sensorData.data1 / 32768 * 16 * 9.81);
        // sBSP_UART_Debug_Printf(" DATA2: %.2f", (float)sensorData.data2 / 32768 * 16 * 9.81);
        // sBSP_UART_Debug_Printf(" DATA3: %.2f", (float)sensorData.data3 / 32768 * 16 * 9.81);
        // sBSP_UART_Debug_Printf(" DATA4: %.2f", (float)sensorData.data4 / 100);
        // sBSP_UART_Debug_Printf(" SUMCRC: 0x%02X\n", sensorData.sum_crc);


        led.toggle();


        HAL_Delay(100);

        

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





