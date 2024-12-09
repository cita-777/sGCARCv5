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
    
    oled.setAll(0);
    oled.handler();

    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

    sLM::MenuItemData p_item = {.id = 1,
                                    .param.access = sLM::Item_ParamAccess::RW_PARAM,
                                    .param.cb = sLM::param_change,
                                    .param.cb_mthd = sLM::Item_ParamCbMethod::CHRG_CB,
                                    .param.i_val = 10,
                                    .param.lim_type = sLM::Item_ParamLimitType::RANGE,
                                    .param.i_dn_val = -10,
                                    .param.i_up_val = 15,
                                    .param.i_increment = 1,
                                    .param.i_decrement = 1,
                                    .text = "P:"};
    sLM::MenuItemData i_item = {.id = 2,
                                    .param.access = sLM::Item_ParamAccess::RW_PARAM,
                                    .param.cb = nullptr,
                                    .param.cb_mthd = sLM::Item_ParamCbMethod::CHRG_CB,
                                    .param.type = sLM::Item_ParamType::INT,
                                    .param.i_val = 0,
                                    .param.lim_type = sLM::Item_ParamLimitType::RANGE,
                                    .param.i_dn_val = -10,
                                    .param.i_up_val = 15,
                                    .param.i_increment = 2,
                                    .param.i_decrement = 2,
                                    .text = "I:"};
    sLM::MenuItemData d_item = {.id = 3,
                                    .param.access = sLM::Item_ParamAccess::RW_PARAM,
                                    .param.cb = nullptr,
                                    .param.cb_mthd = sLM::Item_ParamCbMethod::CHRG_CB,
                                    .param.type = sLM::Item_ParamType::INT,
                                    .param.i_val = 0,
                                    .param.lim_type = sLM::Item_ParamLimitType::RANGE,
                                    .param.i_dn_val = -10,
                                    .param.i_up_val = 15,
                                    .param.i_increment = 2,
                                    .param.i_decrement = 2,
                                    .text = "D:"};
    sLM::MenuItemData bri_item = {.id = 4,
                                    .param.access = sLM::Item_ParamAccess::RW_PARAM,
                                    .param.cb = nullptr,
                                    .param.cb_mthd = sLM::Item_ParamCbMethod::CHRG_CB,
                                    .param.type = sLM::Item_ParamType::INT,
                                    .param.i_val = 0,
                                    .param.lim_type = sLM::Item_ParamLimitType::RANGE,
                                    .param.i_dn_val = -10,
                                    .param.i_up_val = 15,
                                    .param.i_increment = 2,
                                    .param.i_decrement = 2,
                                    .text = "Bri:"};

    sLM::MenuItemData sec_item = {.id = 5,
                                    .param.access = sLM::Item_ParamAccess::RW_PARAM,
                                    .param.cb = nullptr,
                                    .param.cb_mthd = sLM::Item_ParamCbMethod::CHRG_CB,
                                    .param.type = sLM::Item_ParamType::INT,
                                    .param.i_val = 0,
                                    .param.lim_type = sLM::Item_ParamLimitType::RANGE,
                                    .param.i_dn_val = -10,
                                    .param.i_up_val = 15,
                                    .param.i_increment = 2,
                                    .param.i_decrement = 2,
                                    .text = "brightness:"};
    sLM::MenuItemData sec_item2 = {.id = 6,
                                    .param.access = sLM::Item_ParamAccess::RW_PARAM,
                                    .param.cb = nullptr,
                                    .param.cb_mthd = sLM::Item_ParamCbMethod::CHRG_CB,
                                    .param.type = sLM::Item_ParamType::INT,
                                    .param.i_val = 0,
                                    .param.lim_type = sLM::Item_ParamLimitType::RANGE,
                                    .param.i_dn_val = -10,
                                    .param.i_up_val = 15,
                                    .param.i_increment = 2,
                                    .param.i_decrement = 2,
                                    .text = "buzzer:"};
    sLM::MenuItemData sec_item3 = {.id = 7,
                                    .param.access = sLM::Item_ParamAccess::RW_PARAM,
                                    .param.cb = nullptr,
                                    .param.cb_mthd = sLM::Item_ParamCbMethod::CHRG_CB,
                                    .param.type = sLM::Item_ParamType::INT,
                                    .param.i_val = 0,
                                    .param.lim_type = sLM::Item_ParamLimitType::RANGE,
                                    .param.i_dn_val = -10,
                                    .param.i_up_val = 15,
                                    .param.i_increment = 2,
                                    .param.i_decrement = 2,
                                    .text = "time:"};
    sLM::MenuItemData sec_item4 = {.id = 8,
                                    .param.access = sLM::Item_ParamAccess::RW_PARAM,
                                    .param.cb = nullptr,
                                    .param.cb_mthd = sLM::Item_ParamCbMethod::CHRG_CB,
                                    .param.type = sLM::Item_ParamType::INT,
                                    .param.i_val = 0,
                                    .param.lim_type = sLM::Item_ParamLimitType::RANGE,
                                    .param.i_dn_val = -10,
                                    .param.i_up_val = 15,
                                    .param.i_increment = 2,
                                    .param.i_decrement = 2,
                                    .text = "percent:"};


    sLM_TreeNode* p      = sLM_TreeNode::createNode((const void*)&p_item,sizeof(p_item));
    sLM_TreeNode* in  = sLM_TreeNode::createNode((const void*)&i_item,sizeof(i_item));
    sLM_TreeNode* d  = sLM_TreeNode::createNode((const void*)&d_item,sizeof(d_item));
    sLM_TreeNode* bri  = sLM_TreeNode::createNode((const void*)&bri_item,sizeof(bri_item));
    sLM_TreeNode* sec  = sLM_TreeNode::createNode((const void*)&sec_item,sizeof(sec_item));
    sLM_TreeNode* sec2  = sLM_TreeNode::createNode((const void*)&sec_item2,sizeof(sec_item2));
    sLM_TreeNode* sec3  = sLM_TreeNode::createNode((const void*)&sec_item3,sizeof(sec_item3));
    sLM_TreeNode* sec4  = sLM_TreeNode::createNode((const void*)&sec_item4,sizeof(sec_item4));

    lm.init();
    lm.addSubMenu(lm.root,p);
    lm.addSubMenu(lm.root,in);
    lm.addSubMenu(lm.root,d);
    lm.addSubMenu(lm.root,bri);

    lm.addSubMenu(bri,sec);
    lm.addSubMenu(bri,sec2);
    lm.addSubMenu(bri,sec3);
    lm.addSubMenu(bri,sec4);


    oled.setAll(0);
    sLM_Show::showList(lm.root);
    oled.handler();



    
    

    sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

    // 打印菜单结构
    sBSP_UART_Debug_Printf("菜单结构：\n");
    lm.root->printTree(0,printMenuItemData);
    // delete lm.root;


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

        
        HAL_Delay(20);
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





