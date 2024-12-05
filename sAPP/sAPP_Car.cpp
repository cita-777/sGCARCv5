#include "sAPP_Car.hpp"

#include "main.h"


sAPP_Car car;




sAPP_Car::sAPP_Car(){

}

sAPP_Car::~sAPP_Car(){

}




int sAPP_Car::initSys(){
    //初始化HAL库
    HAL_Init();
    //初始化RCC
    sBSP_RCC_Init();
    HAL_InitTick(4);
    //获取时钟频率
    coreClk = HAL_RCC_GetSysClockFreq();
    //初始化DWT
    dwt.init(coreClk);

    //初始化调试串口
    sBSP_UART_Debug_Init(115200);
    //初始化顶层通信串口
    sBSP_UART_Top_Init(115200);
    
    //启用div0异常
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

    //dbg.printf("cm_backtrace初始化完成\n");
    cm_backtrace_init(APPNAME, HARDWARE_VERSION, SOFTWARE_VERSION);

    return 0;
}


int sAPP_Car::initBoard(){
    sBSP_I2C1_Init(400000);

    //初始化IMU
    // imu.init();
    //初始化电机
    motor.init();
    //初始化按键
    sAPP_Btns_Init();
    //初始化二值输出设备
    sAPP_BOD_Init();
    //大功率灯初始化
    sDRV_PL_Init();
    //初始化屏幕
    sDRV_GenOLED_Init();
    oled.init();
    oled.setFPSMode(sG2D::DIGITS2);


    return 0;
}



