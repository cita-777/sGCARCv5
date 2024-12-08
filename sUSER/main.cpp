#include "main.h"



void uart_recied(char* pReciData,uint16_t length){
    sBSP_UART_Debug_SendBytes((uint8_t*)pReciData,length);
    //sBSP_UART_Debug_Printf("%s\n",pReciData);
    //parseSerialData(pReciData);

    sBSP_UART_IMU_RecvBegin(uart_recied);
}


// 计算航向角函数
float calculate_heading(float mag_x, float mag_y) {
    // 使用atan2计算航向角（弧度）
    float heading_rad = atan2(mag_y, mag_x);
    
    // 转换为角度（度数）
    float heading_deg = heading_rad * (180.0 / M_PI);
    
    // 确保角度在0~360范围内
    if (heading_deg < 0) {
        heading_deg += 360.0;
    }

    return heading_deg;
}



sLittleMenu lm;

int main(){
    car.initSys();
    car.initBoard();

    //sBSP_UART_Debug_RecvBegin(uart_recied);
    //sBSP_UART_Top_RecvBegin(uart_recied);

    sBSP_UART_Debug_Printf("STM32 System Clock Freq: %u MHz\n", car.coreClk / 1000000);
    sBSP_UART_Debug_Printf("Hello,STM32F405RGT6    BySightseer.\n");
    sBSP_UART_Debug_Printf("sGCARC初始化完成\n");

    sDRV_MB85RCxx_Init();

    ahrs.init();
    

    // if(sDRV_MB85RCxx_ReadByte(0xFF) != 0xAA){
    //     sBSP_UART_Debug_Printf("IMU还没有进行零偏校准,将在1s后进行校准...\n");
    //     ahrs.calcBias();
    //     float buf[6] = {ahrs.bias_acc_x,ahrs.bias_acc_y,ahrs.bias_acc_z,\
    //                     ahrs.bias_gyro_x,ahrs.bias_gyro_y,ahrs.bias_gyro_z};
    //     sDRV_MB85RCxx_WriteBytes(0x10,(uint8_t*)buf,sizeof(buf));
    //     sDRV_MB85RCxx_WriteByte(0xFF,0xAA);
    //     sBSP_UART_Debug_Printf("校准完成! 校准值:%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",buf[0],buf[1],buf[2],buf[3],\
    //                             buf[4],buf[5]);
    // }else{
    //     float buf[6];
    //     sDRV_MB85RCxx_ReadBytes(0x10,(uint8_t*)buf,sizeof(buf));
    //     sBSP_UART_Debug_Printf("检测到IMU已经校准! 读取的校准值:");
    //     sBSP_UART_Debug_Printf("%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",buf[0],buf[1],buf[2],buf[3],\
    //                             buf[4],buf[5]);
    //     sBSP_UART_Debug_Printf("IMU校准值已应用!\n");
        
    //     ahrs.bias_acc_x = buf[0]; ahrs.bias_acc_y = buf[1]; ahrs.bias_acc_z = buf[2];
    //     ahrs.bias_gyro_x = buf[3]; ahrs.bias_gyro_y = buf[4]; ahrs.bias_gyro_z = buf[5];
    // }
    
    
    

    int i = 0;

    //sAPP_BlcCtrl_Init();

    // sDRV_PS2_Init();
    
    sBSP_RNG_Init();

    sDRV_NTC_Init();

    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

    

    sLM_MenuItemData rootData = {.id = 0,.text = "Root",.show_para_type = sLM_ISPType::STRING,.para_str = "LittleMenu"};
    sLM_MenuItemData settingsData = {.id = 1,.text = "Settings",.show_para_type = sLM_ISPType::STRING,.para_str = "进入"};
    sLM_MenuItemData aboutData = {.id = 2,.text = "About",.show_para_type = sLM_ISPType::STRING,.para_str = "bySightseer"};
    sLM_MenuItemData displayData = {.id = 3,.text = "Display",.show_para_type = sLM_ISPType::FLOAT,.para_float = 17.568};
    sLM_MenuItemData soundData = {.id = 4,.text = "Sound",.show_para_type = sLM_ISPType::INT32_NUM,.para_int = -58};
    sLM_MenuItemData versionData = {.id = 5,.text = "Version",.show_para_type = sLM_ISPType::STRING,.para_str = "v1.0"};

    // 创建菜单节点
    sLM_Menu* root = sLM_Menu::create(&rootData, sizeof(rootData));
    sLM_Menu* settings = sLM_Menu::create(&settingsData, sizeof(settingsData));
    sLM_Menu* about = sLM_Menu::create(&aboutData, sizeof(aboutData));
    sLM_Menu* display = sLM_Menu::create(&displayData, sizeof(displayData));
    sLM_Menu* sound = sLM_Menu::create(&soundData, sizeof(soundData));
    sLM_Menu* version = sLM_Menu::create(&versionData, sizeof(versionData));

    sLM_Menu* display1 = sLM_Menu::create(&displayData, sizeof(displayData));
    sLM_Menu* sound1 = sLM_Menu::create(&soundData, sizeof(soundData));
    sLM_Menu* version1 = sLM_Menu::create(&versionData, sizeof(versionData));

    // // 构建菜单结构
    // root->addSubMenu(settings);
    // root->addSubMenu(about);
    // settings->addSubMenu(display);
    // settings->addSubMenu(sound);
    // about->addSubMenu(version);

    

    lm.init();
    lm.addSub(settings);
    lm.addSub(about);
    lm.addSub(display);
    lm.addSub(sound);
    lm.addSub(version);

    settings->addSubMenu(display1);
    settings->addSubMenu(sound1);
    sound->addSubMenu(version1);

    
    

    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

    // 打印菜单结构
    sBSP_UART_Debug_Printf("菜单结构：\n");
    lm.menu->printMenu(printMenuItemData);

    // 释放资源
    delete lm.menu; // 会递归释放所有节点

    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

    // sAPP_Tasks_CreateAll();
    // sDBG_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
    // sBSP_UART_Debug_Printf("FreeRTOS启动任务调度\n");
    // vTaskStartScheduler();
    

    while(1){
        //处理按键
        sGBD_Handler();
        //处理二值化设备
        BinOutDrv.update();

        
        HAL_Delay(30);
    }
        
    while(1){
        //处理按键
        sGBD_Handler();
        //处理二值化设备
        BinOutDrv.update();
        
        ahrs.update();
        sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f,",ahrs.acc_x,ahrs.acc_y,ahrs.acc_z);
        sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f,",ahrs.gyr_x,ahrs.gyr_y,ahrs.gyr_z);
        sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f,",ahrs.mag_x,ahrs.mag_y,ahrs.mag_z);
        sBSP_UART_Debug_Printf("%6.2f\n",calculate_heading(ahrs.mag_x,ahrs.mag_y));
        // sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f\n",ahrs.pitch,ahrs.roll,ahrs.yaw);


        // sBSP_UART_Debug_Printf("%u\n",sBSP_RNG_GetU8());
        // sBSP_UART_Debug_Printf("%.2f\n",sBSP_RNG_GetRangeFloat(0,100));

        oled.printf(10,50,"%u",i);
        i++;
        dwt.start();
        oled.handler();
        oled.setAll(0);
        dwt.end();
        // sDBG_Debug_Printf("%uus\n",dwt.get_us());

        HAL_Delay(30);
        

    }
}


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





