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



void btn_callback(sLM::ItemBase* item,uint32_t id){
    if(item->getItemType() == sLM::ItemType::BUTTON){
        dbg_printf("OK btn press,id=%u\n",id);

    }
    
}

void int_change_callback(sLM::IntValAdj* item,uint32_t id,int value){
    dbg_printf("OK value=%d,id=%u\n",value,id);

}

void float_change_callback(sLM::FloatValAdj* item,uint32_t id,float value){
    dbg_printf("OK float=%.1f,id=%u\n",value,id);
}

void switch_callback(sLM::SwitchItem* item,uint32_t id,bool status){
    dbg_printf("OK switch=%u,id=%u\n",status?1:0,id);
}

void canvas_enter_callback(sLM::EnterableItem* parent_item,uint32_t id){
    dbg_printf("OK canvas_enter_callback,id=%u\n",id);
}

void canvas_periodically_callback(sLM::EnterableItem* parent_item,uint32_t id){
    // dbg_printf("OK canvas_periodically_callback,id=%u\n",id);
    static uint32_t i = 0;
    i++;
    oled.printf(10,10,"Hello,count=%u",i);
}

void canvas_exit_callback(sLM::EnterableItem* parent_item,uint32_t id){
    dbg_printf("OK canvas_exit_callback,id=%u\n",id);
}

void setup();

int main(){
    car.initSys();
    dbg_printf("----Sightseer's General CAR Controller----\n");
    dbg_printf("%s 硬件版本:%s,特化版本:%s,软件版本:%s\n",APPNAME,HARDWARE_VERSION,SPECIAL_VERSION,SOFTWARE_VERSION);
    car.initBoard();
    dbg_info("sGCARC初始化完成,系统剩余Heap:%u Bytes\n",(uint32_t)xPortGetFreeHeapSize());

    //读取IMU静态零偏
    sAPP_ParamSave_ReadIMUCaliVal();

    // sBSP_UART_Debug_RecvBegin(uart_recied);
    //sBSP_UART_Top_RecvBegin(uart_recied);
    sAPP_BlcCtrl_Init();
    sDRV_PS2_Init();

    dwt.start();
    sAPP_GUI_Init();//创建36个项o0花费306us,o3花费66us

    

    // menu.curr = menu.curr->child;
    menu.operateEnter();
    dwt.end();

    menu.printAllItem();

    // sizeof(FloatValAdj);



    // dwt.start();
    // dwt.end();
    sBSP_UART_Debug_Printf("%uus\n",dwt.get_us());
    // sBSP_UART_Debug_Printf("Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
    
    //! delete内存泄漏问题
    // sBSP_UART_Debug_Printf("DELETE Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
    // delete menu.getHome();
    // sBSP_UART_Debug_Printf("DELETE Current free heap size: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());

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

/*初始化*/
    //using namespace sLM;
    //menu初始化,创建渲染器,绑定到oled,使用menu作为显示菜单
    //menu.init(new OLED128X64(&oled,&menu));

    /*创建可进入的菜单项*/
    //在menu.hone下创建一个可进入的菜单项,ID号为1,标题为"item1"
    //auto* item1 = &EnterableItem::create(menu.home,1).setTittle("Item1");
    
    /*创建一个文本框*/
    //在item4下创建一个文本框,不可进入,id为8,显示标题:"this is label"
    //LabelItem::create(item4,8).setText("this is label");

    /*创建一个按钮,可以按下*/
    //在item4下创建一个可按下的按钮,id为10,显示标题为"a btn",按钮上显示"PRESS",触发事件调用btn_callback
    //ButtonItem::create(item4,10).setContext("a btn","PRESS").setCallback(btn_callback);

    /*创建一个int类型的数值调整项*/
    //在item1下创建一个int类型的数值调整项,id为11,显示标题为"int val",初始默认数值20,触发修改事件调用int_change_callback
    // IntValAdj& int_val = IntValAdj::create(item2,11)
    //     .setCallback(int_change_callback,CallBackMethod::EXIT)
    //     .setContext("int val","%d%%",0,5,5)
    //     .setConstraint(ConstraintType::RANGE,10,-10);

    // auto* item1 = &EnterableItem::create(menu.home,1).setTittle("Item1");
    // auto* item2 = &EnterableItem::create(menu.home,2).setTittle("Item2");
    // auto* item3 = &EnterableItem::create(menu.home,3).setTittle("Item3");
    // auto* item4 = &EnterableItem::create(menu.home,4).setTittle("Item4");

    // auto* item5 = &EnterableItem::create(menu.home,5).setTittle("Item5");
    // auto* item7 = &EnterableItem::create(menu.home,7).setTittle("Item7");

    // LabelItem::create(item4,8).setTittle("this is label");

    // ButtonItem::create(item4,10).setContext("a btn","PRESS").setCallback(btn_callback);

    // IntValAdj& int_val = IntValAdj::create(item2,11)
    //     .setCallback(int_change_callback,CallBackMethod::EXIT)
    //     .setContext("int val","%d%%",0,5,5)
    //     .setConstraint(ConstraintType::RANGE,10,-10);

    // FloatValAdj& float_val = FloatValAdj::create(item2,12)
    //     .setCallback(float_change_callback,CallBackMethod::CHANGE)
    //     .setContext("float",nullptr,2.4,0.1,0.1)
    //     .setConstraint(ConstraintType::RANGE,3,-3);

    // SwitchItem& switch1 = SwitchItem::create(item2,13)
    //     .setContext("a switch")
    //     .setCallback(switch_callback);

    // auto* canvas_item = &EnterableItem::create(menu.home,14).setTittle("Canvas Item")
    //     .setChildShowType(ItemShowType::CANVAS)
    //     .setCanvasEnterCallback(canvas_enter_callback)
    //     .setCanvasPeriodicallyCallback(canvas_periodically_callback)
    //     .setCanvasExitCallback(canvas_exit_callback);


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





