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
        // ps2.leftX = (uint8_t)t_leftX;
        // ps2.leftY = (uint8_t)t_leftY;
        // ps2.rightX = (uint8_t)t_rightX;
        // ps2.rightY = (uint8_t)t_rightY;
                            // ps2.cir  = (u_cir  != 0);
                            // ps2.rect = (u_rect != 0);
                            // ps2.tri  = (u_tri  != 0);
                            // ps2.x    = (u_x    != 0);
// sBSP_UART_Debug_Printf("OK\n");
// sBSP_UART_Debug_Printf("0x%2X,0x%2X\n", ps2.leftY,ps2.rightX);
                        }

    sBSP_UART_Top_RecvBegin(uart_recied);
}





void setup();

int main(){
    car.initSys();
    // dbg_printf("STM32 System Clock Freq: %u MHz\n", car.coreClock / 1000000);
    dbg_printf("----Sightseer's General CAR Controller----\n");
    dbg_printf("%s 硬件版本:%s,特化版本:%s,软件版本:%s\n",APPNAME,HARDWARE_VERSION,SPECIAL_VERSION,SOFTWARE_VERSION);
    HAL_Delay(20); //等待上电稳定
    car.initBoard();
    dbg_info("sGCARC初始化完成,系统剩余Heap:%u Bytes\n",(uint32_t)xPortGetFreeHeapSize());

    HAL_Delay(10);

    //读取IMU静态零偏
    sAPP_ParamSave_ReadIMUCaliVal();

    // sBSP_UART_Debug_RecvBegin(uart_recied);
    //sBSP_UART_Top_RecvBegin(uart_recied);
    sAPP_BlcCtrl_Init();
    sDRV_PS2_Init();
    
    using namespace sLM;
    // sAPP_GUI_Init();

    menu.init(new OLED128X64(&oled,&menu));
    // menu.init();


    auto item1 = menu.createEnterable(menu.home,"item1");
    auto item2 = menu.createEnterable(menu.home,"item2");
    auto item3 = menu.createEnterable(menu.home,"item3");
    auto item4 = menu.createEnterable(menu.home,"item4");

    auto item5 = menu.createEnterable(item4,"item5");
    auto item6 = menu.createEnterable(item4,"item6");
    auto item7 = menu.createEnterable(item2,"item7");

    menu.curr = menu.curr->child;

    // dwt.start();
    // dwt.end();
    // sBSP_UART_Debug_Printf("%uus\n",dwt.get_us());
    // sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

    // 打印菜单结构
    // sBSP_UART_Debug_Printf("菜单结构：\n");
    // menu.getRoot()->printTree(0,sLM::printItemData);
    // delete menu.getRoot();

    // sBSP_UART_Debug_Printf("menu id count:%u\n",menu.getItemCount());
    // sBSP_UART_Debug_Printf("menu 21 item text:%s\n",sLM::sLittleMenu::getNodeData(menu.getItemByID(21)).text);
    

    setup();
    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

    sAPP_Tasks_CreateAll();
    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
    sBSP_UART_Debug_Printf("FreeRTOS启动任务调度\n");
    vTaskStartScheduler();
    // while(1);
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

    delay(30);
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





