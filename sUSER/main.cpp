#include "main.h"



//128+64K SRAM
//1024K FLASH
//168MHz 210DMIPS

//compile env:
//AC6 C++14


/**
 * 完成TOP UART串口驱动 DMA收发
 * 已完成:串口1 DMA+空闲中断接收 241031
 * 
 * 完成PS2驱动 
 * 完成sBD
 * 完成sGraphic2D v2 脏区块刷新 简单非线性动画
 * 
 * 电池电量报警
 * 
 * Matlab 磁力计校准算法 自动控制 联调 
 * 
 * FeRAM应用 人
 * 
 * 
 */



#include <stdio.h>


void parseSerialData(const char* data);

void uart_recied(char* pReciData,uint16_t length){
    sBSP_UART_Debug_SendBytes((uint8_t*)pReciData,length);
    //sBSP_UART_Debug_Printf("%s\n",pReciData);
    //parseSerialData(pReciData);

    sBSP_UART_IMU_RecvBegin(uart_recied);
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

// 假设全局变量结构体
typedef struct {
    uint8_t type;        // TYPE
    short data1;         // DATA1
    short data2;         // DATA2
    short data3;         // DATA3
    short data4;         // DATA4
    uint8_t sum_crc;     // 校验和 SUMCRC
} SensorData;

SensorData sensorData;  // 用来存储解析后的数据

// 函数：解析串口数据并更新全局变量
void parseSerialData(const char* data) {
    // 假设数据为16进制字符串形式，且长度符合要求

    // 提取各个字段
    uint8_t type = data[1];  // TYPE字段（数据包的第2个字节）
    uint8_t data1L = data[2];  // DATA1低字节
    uint8_t data1H = data[3];  // DATA1高字节
    uint8_t data2L = data[4];  // DATA2低字节
    uint8_t data2H = data[5];  // DATA2高字节
    uint8_t data3L = data[6];  // DATA3低字节
    uint8_t data3H = data[7];  // DATA3高字节
    uint8_t data4L = data[8];  // DATA4低字节
    uint8_t data4H = data[9];  // DATA4高字节
    uint8_t sum_crc = data[10];  // SUMCRC字段（校验和）

    // 计算DATA1, DATA2, DATA3, DATA4（16位带符号值）
    short data1 = (short)((short)data1H << 8 | data1L);  // 合并低字节和高字节，得到有符号的short
    short data2 = (short)((short)data2H << 8 | data2L);
    short data3 = (short)((short)data3H << 8 | data3L);
    short data4 = (short)((short)data4H << 8 | data4L);

    // 更新全局变量
    sensorData.type = type;
    sensorData.data1 = data1;
    sensorData.data2 = data2;
    sensorData.data3 = data3;
    sensorData.data4 = data4;
    sensorData.sum_crc = sum_crc;

    sBSP_UART_Debug_Printf("TYPE: 0x%02X,%d,%d,%d,%d,%d\n",\
    sensorData.type,sensorData.data1,sensorData.data2,sensorData.data3,sensorData.data4,sensorData.sum_crc);

}


int main(){
    car.initSys();
    car.initBoard();

    //sBSP_UART_Debug_RecvBegin(uart_recied);
    //sBSP_UART_Top_RecvBegin(uart_recied);

    sBSP_UART_Debug_Printf("STM32 System Clock Freq: %u MHz\n", car.coreClk / 1000000);
    sBSP_UART_Debug_Printf("Hello,STM32F405RGT6    BySightseer.\n");
    sBSP_UART_Debug_Printf("sGCARCv4初始化完成\n");

    sDRV_GenOLED_Init();
    oled.init();
    oled.setFPSMode(sG2D::DIGITS2);

    
    sDRV_JY901S_Init();
    sBSP_UART_IMU_RecvBegin(uart_recied);




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






