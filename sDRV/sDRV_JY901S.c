#include "sDRV_JY901S.h"

/**
 * sDRV_JY901S.c
 * 维特智能的JY901S IMU驱动
 * 使用UART通信
 * 参考:https://wit-motion.yuque.com/wumwnr/ltst03/vl3tpy?#S6al6
 * 
 * 241115 v1.0
 * 第一版 bySightseer. inHNIP 9607Lab
 */


/*******************************************接口*******************************************/

static void portUARTInit(uint32_t bandrate){
    sBSP_UART_IMU_Init(bandrate);
}
static void portSendByte(uint8_t data){
    sBSP_UART_IMU_SendByte(data);
}
static uint8_t portRecvByte(){
    return 0;
}

#include "sBSP_UART.h"

static void sendBytes(const char* pData,uint8_t len){
    //sBSP_UART_Debug_SendBytes((uint8_t*)pData,len);
    sBSP_UART_IMU_SendBytes((uint8_t*)pData,len);
}


//前面无下划线:无参宏 有一个下划线:带参宏,有两个:功能性宏,有三个:用于中间操作,用户不要调用

//解锁
#define COMM_UNLOCK                 "\xFF\xAA\x69\x88\xB5"
//保存   
#define ___COMM_SAVE(__VAL__)        "\xFF\xAA\x00"__VAL__"\x00"
#define _COMM_SAVE(__VAL__) ___COMM_SAVE(__VAL__)

//重启   
#define COMM_REBOOT          "\xFF\xAA\x00\xFF\x00"

//设置波特率115200
//! 注意,这里没有用到设置波特率,因为之前波特率和现在不一样的话,发过去IMU没办法解包,所以要先在上位机上设置好波特率
#define COMM_BANDRATE_115200 "\xFF\xAA\x04\x06\x00"

//设置输出内容为:加速度 角速度 角度 磁场 四元数
#define COMM_RSW_IMU         "\xFF\xAA\x02\x1E\x02"

//设置输出速度 0.2Hz
#define COMM_RRATE_0D2HZ     "\xFF\xAA\x03\x01\x00"
#define COMM_RRATE_0D5HZ     "\xFF\xAA\x03\x02\x00"
#define COMM_RRATE_1HZ       "\xFF\xAA\x03\x03\x00"
#define COMM_RRATE_10HZ      "\xFF\xAA\x03\x06\x00"

//构造命令
#define ___COMM(__ADDR__, __VAL__) "\xFF\xAA" __ADDR__ __VAL__ "\x00"
#define _COMM(__ADDR__, __VAL__)   ___COMM(__ADDR__, __VAL__)

//获取指令长度, -1是为了跳过\n
#define __COMM_LEN(__COMM__)       (sizeof(__COMM__) - 1)


int sDRV_JY901S_Init(){
    //! 注意先在上位机上配置好波特率115200
    portUARTInit(115200);

    //解锁
    sendBytes(COMM_UNLOCK,__COMM_LEN(COMM_UNLOCK));
    //等待200ms
    HAL_Delay(200);

    //配置输出信息
    sendBytes(COMM_RSW_IMU,__COMM_LEN(COMM_RSW_IMU));
    //配置输出
    sendBytes(COMM_RRATE_1HZ,__COMM_LEN(COMM_RRATE_1HZ));

    //等待200ms
    HAL_Delay(200);

    //保存
    sendBytes(_COMM_SAVE(JY901S_SAVE_SAVE),__COMM_LEN(_COMM_SAVE(JY901S_SAVE_SAVE)));


    return 0;
}

void sDRV_JY901S_SetBandrate115200(){
    sendBytes(COMM_UNLOCK,__COMM_LEN(COMM_UNLOCK));
    HAL_Delay(200);
    sendBytes(COMM_BANDRATE_115200,__COMM_LEN(COMM_BANDRATE_115200));

    portUARTInit(115200);
    sendBytes(COMM_UNLOCK,__COMM_LEN(COMM_UNLOCK));
    HAL_Delay(200);
    sendBytes(_COMM_SAVE(JY901S_SAVE_SAVE),__COMM_LEN(_COMM_SAVE(JY901S_SAVE_SAVE)));
}

void sDRV_JY901S_SetRRATE10Hz(){
    //解锁
    sendBytes(COMM_UNLOCK,__COMM_LEN(COMM_UNLOCK));
    //等待200ms
    HAL_Delay(200);
    //配置输出
    sendBytes(COMM_RRATE_1HZ,__COMM_LEN(COMM_RRATE_10HZ));
    //等待200ms
    HAL_Delay(200);
    //保存
    sendBytes(_COMM_SAVE(JY901S_SAVE_SAVE),__COMM_LEN(_COMM_SAVE(JY901S_SAVE_SAVE)));
}


void sDRV_JY901S_Handler(){

}



