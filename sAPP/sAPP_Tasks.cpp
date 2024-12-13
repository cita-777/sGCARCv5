#include "sAPP_Tasks.hpp"

#include "main.h"




void sAPP_Tasks_ControlAlgo(void* param){
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    for(;;){
        motor.update();
        sAPP_BlcCtrl_Handler();
        

        //高精确度延时10ms
        xTaskDelayUntil(&xLastWakeTime,10 / portTICK_PERIOD_MS);
    }
}

void sAPP_Tasks_AHRS(void* param){
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    for(;;){
        ahrs.update();
        
        

        // sBSP_UART_Debug_Printf("pitch: %6.2f, roll: %6.2f, yaw: %6.2f\n",ahrs.pitch,ahrs.roll,ahrs.yaw);

        //高精确度延时10ms
        xTaskDelayUntil(&xLastWakeTime,10 / portTICK_PERIOD_MS);
    }
}

void sAPP_Tasks_OLEDHdr(void* param){
    for(;;){
        slm_weights.showList(slm.curr_page->parent);
        oled.handler();
        oled.setAll(0);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


void sAPP_Tasks_Devices(void* param){
    for(;;){
        //处理按键
        sGBD_Handler();
        //处理二值化设备
        BinOutDrv.update();


        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}


void sAPP_Tasks_500ms(void* param){

    for(;;){

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void sAPP_Tasks_1000ms(void* param){

    for(;;){

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void sAPP_Tasks_TaskMang(void* param){

    for(;;){
        sAPP_Tasks_PrintTaskMang();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}







void sAPP_Tasks_CreateAll(){
    //控制算法 100Hz
    xTaskCreate(sAPP_Tasks_ControlAlgo  , "ControlAlgo"  , 2048 / sizeof(int), NULL, 4, NULL);
    //姿态估计算法 100Hz  
    xTaskCreate(sAPP_Tasks_AHRS         , "AHRS"         ,  512 / sizeof(int), NULL, 3, NULL);
    //OLED刷屏 20Hz  
    xTaskCreate(sAPP_Tasks_OLEDHdr      , "OLED"         , 2048 / sizeof(int), NULL, 1, NULL);
    xTaskCreate(sAPP_Tasks_Devices      , "Devices"      ,  512 / sizeof(int), NULL, 1, NULL);
    xTaskCreate(sAPP_Tasks_500ms        , "500ms"        ,  512 / sizeof(int), NULL, 1, NULL);
    xTaskCreate(sAPP_Tasks_1000ms       , "1000ms"       ,  512 / sizeof(int), NULL, 1, NULL);
    //xTaskCreate(sAPP_Tasks_TaskMang     , "TaskMang"     , 2048 / sizeof(int), NULL, 1, NULL);
}



void sAPP_Tasks_PrintTaskMang(){
    const char task_state[]={'r','R','B','S','D'};
    volatile UBaseType_t uxArraySize, x;
    unsigned portSHORT ulTotalRunTime,ulStatsAsPercentage;
    /* 获取任务总数目 */
    uxArraySize = uxTaskGetNumberOfTasks();
    sBSP_UART_Debug_Printf("当前任务数量:%d\n",uxArraySize);
    TaskStatus_t pxTaskStatusArray[16];
    /*获取每个任务的状态信息 */
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray,uxArraySize,&ulTotalRunTime);
    sBSP_UART_Debug_Printf("任务名        状态       ID       优先级       堆栈剩余      CPU使用率\n");
    /* 避免除零错误 */
    if( ulTotalRunTime > 0 ){
        /* 将获得的每一个任务状态信息部分的转化为程序员容易识别的字符串格式 */
        for(x = 0; x < uxArraySize; x++){
            char tmp[128];
            /* 计算任务运行时间与总运行时间的百分比。*/
            ulStatsAsPercentage =(uint16_t)(pxTaskStatusArray[x].ulRunTimeCounter)*100 / ulTotalRunTime;
 
            if( ulStatsAsPercentage > 0UL ){
               sprintf(tmp,"%-15s%-10c%-10lu%-12lu%-12d%d%%",pxTaskStatusArray[ x].pcTaskName,task_state[pxTaskStatusArray[ x ].eCurrentState],
                                                             pxTaskStatusArray[ x ].xTaskNumber,pxTaskStatusArray[ x].uxCurrentPriority,
                                                             pxTaskStatusArray[ x ].usStackHighWaterMark,ulStatsAsPercentage);
            }
            else{
                /* 任务运行时间不足总运行时间的1%*/
                sprintf(tmp,"%-15s%-10c%-10lu%-12lu%-12dt<1%%",pxTaskStatusArray[x ].pcTaskName,task_state[pxTaskStatusArray[ x ].eCurrentState],
                                                               pxTaskStatusArray[ x ].xTaskNumber,pxTaskStatusArray[ x].uxCurrentPriority,
                                                               pxTaskStatusArray[ x ].usStackHighWaterMark);               
            }
           sBSP_UART_Debug_Printf("%s\n",tmp);
        }
    }
    sBSP_UART_Debug_Printf("任务状态:   r-运行  R-就绪  B-阻塞  S-挂起  D-删除\n");
}








