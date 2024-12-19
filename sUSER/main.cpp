#include "main.h"



void uart_recied(char* pReciData,uint16_t length){
    // sBSP_UART_Debug_SendBytes((uint8_t*)pReciData,length);
    unsigned u_cir, u_rect, u_tri, u_x; // 暂存无符号整型数
    int t_leftX, t_leftY, t_rightX, t_rightY;
    
    //sBSP_UART_Debug_Printf("%s\n",pReciData);
    //parseSerialData(pReciData);
    if(sscanf(pReciData, "S:%2X,%2X,%2X,%2X,%u,%u,%u,%u:E",
                        &t_leftX, &t_leftY, &t_rightX, &t_rightY,
                        &u_cir, &u_rect, &u_tri, &u_x) == 8){
                            // 转换为uint8_t类型
        ps2.leftX = (uint8_t)t_leftX;
        ps2.leftY = (uint8_t)t_leftY;
        ps2.rightX = (uint8_t)t_rightX;
        ps2.rightY = (uint8_t)t_rightY;
                            ps2.cir  = (u_cir  != 0);
                            ps2.rect = (u_rect != 0);
                            ps2.tri  = (u_tri  != 0);
                            ps2.x    = (u_x    != 0);
// sBSP_UART_Debug_Printf("OK\n");
sBSP_UART_Debug_Printf("0x%2X,0x%2X\n", ps2.leftY,ps2.rightX);
                        }

    sBSP_UART_Top_RecvBegin(uart_recied);
}



void setup();

int main(){
    car.initSys();
    car.initBoard();

    sBSP_UART_Debug_Printf("STM32 System Clock Freq: %u MHz\n", car.coreClk / 1000000);
    sBSP_UART_Debug_Printf("Hello,STM32F405RGT6    BySightseer.\n");
    sBSP_UART_Debug_Printf("sGCARC初始化完成\n");

    sAPP_ParamSave_ReadIMUCaliVal();

    // sBSP_UART_Debug_RecvBegin(uart_recied);
    sBSP_UART_Top_RecvBegin(uart_recied);


    int i = 0;

    sAPP_BlcCtrl_Init();

    sDRV_PS2_Init();
    
    oled.setAll(0);
    oled.handler();

    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
    sAPP_GUI_Init();
    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

    // 打印菜单结构
    sBSP_UART_Debug_Printf("菜单结构：\n");
    slm.root->printTree(0,printMenuItemData);
    // delete slm.root;

    setup();

    sAPP_Tasks_CreateAll();
    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
    sBSP_UART_Debug_Printf("FreeRTOS启动任务调度\n");
    vTaskStartScheduler();
}







void setup(){

}



void loop(){
    // sDRV_PS2_Handler();
    // sBSP_UART_Debug_Printf("0x%2X,0x%2X\n", ps2.leftX,ps2.leftY);
    // sBSP_UART_Debug_Printf("S:%2X,%2X,%2X,%2X,%u,%u,%u,%u:E\n", ps2.leftX,ps2.leftY,ps2.rightX,ps2.rightY,ps2.cir,ps2.rect,ps2.tri,ps2.x);
    // sBSP_UART_Debug_Printf("%.2f,%.2f\n",motor.getLRPM(),motor.getRRPM());
    // motor.setLM(100);
    // motor.setRM(100);

    delay(20);
}


//10min 偏1.5度




/**
 *     while(1){
        //处理按键
        sGBD_Handler();
        //处理二值化设备
        BinOutDrv.update();
        
        ahrs.update();
        // sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f,",ahrs.acc_x,ahrs.acc_y,ahrs.acc_z);
        // sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f,",ahrs.gyr_x,ahrs.gyr_y,ahrs.gyr_z);
        // sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f,",ahrs.mag_x,ahrs.mag_y,ahrs.mag_z);
        // sBSP_UART_Debug_Printf("%6.2f\n",calculate_heading(ahrs.mag_x,ahrs.mag_y));
        // sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f\n",ahrs.pitch,ahrs.roll,ahrs.yaw);


        // sBSP_UART_Debug_Printf("%u\n",sBSP_RNG_GetU8());
        // sBSP_UART_Debug_Printf("%.2f\n",sBSP_RNG_GetRangeFloat(0,100));

        oled.printf(10,50,"%u",i);
        i++;
        dwt.start();
        oled.handler();
        oled.setAll(0);
        dwt.end();
        // sBSP_UART_Debug_Printf("%uus\n",dwt.get_us());

        HAL_Delay(30);
    }
 */


/*用于重载c++ new/delete分配器,让其指向FreeRTOS的内存管理策略*/

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





