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
    sBSP_UART_Debug_Printf("Send:0x%02X 0x%02X 0x%02X 0x%02X 0x%02X Len:%u \n"\
    ,pData[0],pData[1],pData[2],pData[3],pData[4],len);
}


//前面无下划线:无参宏 有一个下划线:带参宏,有两个:功能性宏,有三个:用于中间操作,用户不要调用

//SAVE 保存/重启/恢复出厂 RW
#define ADDR_SAVE           "\x00"
#define SAVE_SAVE           "\x00\x00"
#define SAVE_REBOOT         "\xFF\x00"
#define SAVE_RESTORE        "\x01\x00"

//KEY 解锁
#define ADDR_KEY            "\x69"
//写0xB588解锁
#define KEY_KEY             "\x88\xB5"

//CALSW 校准模式 RW ADDR:0x01 
#define ADDR_CALSW          "\x01\x00"
#define CALSW_NORMAL        "\x00\x00" //正常工作模式
#define CALSW_AUTOCAL       "\x01\x00" //自动加计校准
#define CALSW_ALTCLR        "\x03\x00" //高度清零
#define CALSW_HEADINGCLR    "\x04\x00" //航向角置0
#define CALSW_MAGCAL1       "\x07\x00" //磁场校准(球形拟合法)
#define CALSW_SETDEGREF     "\x08\x00" //设置角度参考
#define CALSW_MAGCAL2       "\x09\x00" //磁场校准(双平面模式)

//RSW 输出内容 RW ADDR:0x02
#define ADDR_RSW            "\x02"
//设置输出内容为:加速度 角速度 角度 磁场 四元数
#define RSW_IMU             "\x1E\x02"
#define RSW_GSA             "\x04\x00" //0x01 << 10 = 0x0400
#define RSW_QUATER          "\x02\x00" //0x01 << 9  = 0x0200
#define RSW_VELOCITY        "\x01\x00" //0x01 << 8  = 0x0100
#define RSW_GPS             "\x00\x80" //0x01 << 7  = 0x0080
#define RSW_PRESS           "\x00\x40" //0x01 << 6  = 0x0040
#define RSW_PORT            "\x00\x20" //0x01 << 5  = 0x0020
#define RSW_MAG             "\x00\x10" //0x01 << 4  = 0x0010
#define RSW_ANGLE           "\x00\x08" //0x01 << 3  = 0x0008
#define RSW_GYRO            "\x00\x04" //0x01 << 2  = 0x0004
#define RSW_ACC             "\x00\x02" //0x01 << 1  = 0x0002
#define RSW_TIME            "\x00\x01" //0x01 << 0  = 0x0001

//RRATE 输出速率寄存器 ADDR:0x03
#define ADDR_RRATE          "\x03"
#define RRATE_0D2HZ         "\x01\x00"
#define RRATE_0D5HZ         "\x02\x00"
#define RRATE_1HZ           "\x03\x00"
#define RRATE_2HZ           "\x04\x00"
#define RRATE_5HZ           "\x05\x00"
#define RRATE_10HZ          "\x06\x00"
#define RRATE_20HZ          "\x07\x00"
#define RRATE_50HZ          "\x08\x00"
#define RRATE_100HZ         "\x09\x00"
#define RRATE_200HZ         "\x0B\x00"
#define RRATE_ONCE          "\x0C\x00" //单次回传
#define RRATE_NEVER         "\x0D\x00" //不回传

//BANDWIDTH 带宽 RW ADDR:0x1F 默认20Hz
#define ADDR_BANDWIDTH      "\x1F"
#define BANDWIDTH_256HZ     "\x00\x00"
#define BANDWIDTH_188HZ     "\x01\x00"
#define BANDWIDTH_98HZ      "\x02\x00"
#define BANDWIDTH_42HZ      "\x03\x00"
#define BANDWIDTH_20HZ      "\x04\x00"
#define BANDWIDTH_10HZ      "\x05\x00"
#define BANDWIDTH_5HZ       "\x06\x00"

//AXIS6 算法 RW ADDR:0x24
#define ADDR_AXIS6          "\x24"
#define AXIS6_9AXIS         "\x00\x00" //9轴算法 绝对航向角
#define AXIS6_6AXIS         "\x01\x00" //6轴算法 相对航向角


//保存   
#define _COMM_SAVE(__VAL__) "\xFF\xAA\x00"__VAL__"\x00"

//重启   
#define COMM_REBOOT          "\xFF\xAA\x00\xFF\x00"

//设置波特率115200
//! 注意,这里没有用到设置波特率,因为之前波特率和现在不一样的话,发过去IMU没办法解包,所以要先在上位机上设置好波特率
#define COMM_BANDRATE_115200 "\xFF\xAA\x04\x06\x00"



//构造命令
#define _COMM(__ADDR__, __VAL__)     "\xFF\xAA"__ADDR__ __VAL__
//获取指令长度, -1是为了跳过\n
#define __COMM_LEN(__COMM__)         (sizeof(__COMM__) - 1)

#define __SEND_COMM_UNLOCK(__X__)       sendBytes(_COMM(ADDR_KEY,KEY_KEY),__COMM_LEN(_COMM(ADDR_KEY,KEY_KEY)));
#define __SEND_COMM_SAVE(__X__)         sendBytes(_COMM(ADDR_SAVE,SAVE_SAVE),__COMM_LEN(_COMM(ADDR_SAVE,SAVE_SAVE)));

#define __SEND_COMM_RRATE10HZ(__X__)    sendBytes(_COMM(ADDR_RRATE,RRATE_10HZ),__COMM_LEN(_COMM(ADDR_RRATE,RRATE_10HZ)));
#define __SEND_COMM_RRATENEVER(__X__)   sendBytes(_COMM(ADDR_RRATE,RRATE_NEVER),__COMM_LEN(_COMM(ADDR_RRATE,RRATE_NEVER)));


#define __SEND_COMM_CONF_IMU(__X__)     sendBytes(_COMM(ADDR_RSW,RSW_IMU),__COMM_LEN(_COMM(ADDR_RSW,RSW_IMU)));

#define __SEND_COMM_AXIS6(__X__)        sendBytes(_COMM(ADDR_AXIS6,AXIS6_6AXIS),__COMM_LEN(_COMM(ADDR_AXIS6,AXIS6_6AXIS)));
#define __SEND_COMM_AXIS9(__X__)        sendBytes(_COMM(ADDR_AXIS6,AXIS6_9AXIS),__COMM_LEN(_COMM(ADDR_AXIS6,AXIS6_9AXIS)));


int sDRV_JY901S_Init(){
    //! 注意先在上位机上配置好波特率115200
    portUARTInit(115200);

    HAL_Delay(200);


    //解锁
    __SEND_COMM_UNLOCK();
    //等待200ms
    HAL_Delay(200);
    //配置输出
    //__SEND_COMM_CONF_IMU();
    //首先不回传,等我空闲中断启动之后再回传
    //__SEND_COMM_RRATENEVER();
    __SEND_COMM_AXIS9();

    //等待200ms
    HAL_Delay(200);
    //保存
    __SEND_COMM_SAVE();
    HAL_Delay(200);

    return 0;
}

void sDRV_JY901S_SetBandrate115200(){
    __SEND_COMM_UNLOCK();
    HAL_Delay(200);
    sendBytes(COMM_BANDRATE_115200,__COMM_LEN(COMM_BANDRATE_115200));

    portUARTInit(115200);
    __SEND_COMM_UNLOCK();
    HAL_Delay(200);
    __SEND_COMM_SAVE();
}

void sDRV_JY901S_SetRRATENever(){
    //解锁
    __SEND_COMM_UNLOCK();
    //等待200ms
    HAL_Delay(200);
    //配置输出
    __SEND_COMM_RRATENEVER();
    //等待200ms
    HAL_Delay(200);
    //保存
    __SEND_COMM_SAVE();
}

void sDRV_JY901S_SetRRATE10Hz(){
    //解锁
    __SEND_COMM_UNLOCK();
    //等待200ms
    HAL_Delay(200);
    //配置输出
    __SEND_COMM_RRATE10HZ();
    //等待200ms
    HAL_Delay(200);
    //保存
    __SEND_COMM_SAVE();
}

// 假设全局变量结构体
typedef struct {
    uint8_t type;        // TYPE
    int16_t data1;         // DATA1
    int16_t data2;         // DATA2
    int16_t data3;         // DATA3
    int16_t data4;         // DATA4
    uint8_t sum_crc;     // 校验和 SUMCRC
} data_t;

typedef struct{
    float acc_x, acc_y, acc_z;
    float gyr_x, gyr_y, gyr_z;
    float pitch, roll , yaw;
    float mag_x, mag_y, mag_z;
    float q0, q1, q2, q3;
    float temp;
}package_t;

static package_t pkg;

static data_t data_acc;
static data_t data_gyr;
static data_t data_deg;
static data_t data_mag;
static data_t data_deg;
static data_t data_qua;

void sDRV_JY901S_Handler(char* pReciData,uint16_t length){

    if(length != 55){
        //sBSP_UART_Debug_Printf("length:%u\n",length);
        goto END;
        return;
    }
    
    //解析IMU数据包,注意,RSW格式
    uint16_t offset = 0;

    //0x51 ACCEL
    data_acc.type    =  pReciData[offset + 1];
    data_acc.data1   = (pReciData[offset + 3] << 8) | pReciData[offset + 2];
    data_acc.data2   = (pReciData[offset + 5] << 8) | pReciData[offset + 4];
    data_acc.data3   = (pReciData[offset + 7] << 8) | pReciData[offset + 6];
    data_acc.data4   = (pReciData[offset + 9] << 8) | pReciData[offset + 8];
    data_acc.sum_crc = pReciData[offset + 10];
    offset += 11;
    //0x52 GYRO
    data_gyr.type    =  pReciData[offset + 1];
    data_gyr.data1   = (pReciData[offset + 3] << 8) | pReciData[offset + 2];
    data_gyr.data2   = (pReciData[offset + 5] << 8) | pReciData[offset + 4];
    data_gyr.data3   = (pReciData[offset + 7] << 8) | pReciData[offset + 6];
    data_gyr.data4   = (pReciData[offset + 9] << 8) | pReciData[offset + 8];
    data_gyr.sum_crc = pReciData[offset + 10];
    offset += 11;
    //0x53 Deg
    data_deg.type    =  pReciData[offset + 1];
    data_deg.data1   = (pReciData[offset + 3] << 8) | pReciData[offset + 2];
    data_deg.data2   = (pReciData[offset + 5] << 8) | pReciData[offset + 4];
    data_deg.data3   = (pReciData[offset + 7] << 8) | pReciData[offset + 6];
    data_deg.data4   = (pReciData[offset + 9] << 8) | pReciData[offset + 8];
    data_deg.sum_crc = pReciData[offset + 10];
    offset += 11;
    //0x53 mGa
    data_mag.type    =  pReciData[offset + 1];
    data_mag.data1   = (pReciData[offset + 3] << 8) | pReciData[offset + 2];
    data_mag.data2   = (pReciData[offset + 5] << 8) | pReciData[offset + 4];
    data_mag.data3   = (pReciData[offset + 7] << 8) | pReciData[offset + 6];
    data_mag.data4   = (pReciData[offset + 9] << 8) | pReciData[offset + 8];
    data_mag.sum_crc = pReciData[offset + 10];
    offset += 11;
    //0x53 mGa
    data_qua.type    =  pReciData[offset + 1];
    data_qua.data1   = (pReciData[offset + 3] << 8) | pReciData[offset + 2];
    data_qua.data2   = (pReciData[offset + 5] << 8) | pReciData[offset + 4];
    data_qua.data3   = (pReciData[offset + 7] << 8) | pReciData[offset + 6];
    data_qua.data4   = (pReciData[offset + 9] << 8) | pReciData[offset + 8];
    data_qua.sum_crc = pReciData[offset + 10];
    offset += 11;
    
    //m/s^2
    pkg.acc_x = (float)data_acc.data1 / 32768.0f * 16.0f * 9.81f;
    pkg.acc_y = (float)data_acc.data2 / 32768.0f * 16.0f * 9.81f;
    pkg.acc_z = (float)data_acc.data3 / 32768.0f * 16.0f * 9.81f;
    pkg.temp  = (float)data_acc.data4 / 100.0f;
    //deg
    pkg.gyr_x = (float)data_gyr.data1 / 32768.0f * 2000.0f;
    pkg.gyr_y = (float)data_gyr.data2 / 32768.0f * 2000.0f;
    pkg.gyr_z = (float)data_gyr.data3 / 32768.0f * 2000.0f;
    //deg
    pkg.roll  = (float)data_deg.data1 / 32768.0f * 180.0f;
    pkg.pitch = (float)data_deg.data2 / 32768.0f * 180.0f;
    pkg.yaw   = (float)data_deg.data3 / 32768.0f * 180.0f;
    //mag
    pkg.mag_x = (float)data_mag.data1;
    pkg.mag_y = (float)data_mag.data2;
    pkg.mag_z = (float)data_mag.data3;
    //quaternion
    pkg.q0    = (float)data_qua.data1;
    pkg.q1    = (float)data_qua.data2;
    pkg.q2    = (float)data_qua.data3;
    pkg.q3    = (float)data_qua.data4;




    sBSP_UART_Debug_Printf("ax:%.2f,ay:%.2f,az:%.2f,gx:%.2f,gy:%.2f,gz:%.2f,p:%.2f,r:%.2f,y:%.2f,mx:%.2f,my:%.2f,mz:%.2f\n",\
    pkg.acc_x,pkg.acc_y,pkg.acc_z,\
    pkg.gyr_x,pkg.gyr_y,pkg.gyr_z,\
    pkg.pitch,pkg.roll,pkg.yaw,\
    pkg.mag_x,pkg.mag_y,pkg.mag_z);


    // sBSP_UART_Debug_Printf("TYPE: 0x%02X,%d,%d,%d,%d,%d\n",\
    // data1.type,data1.data1,data1.data2,data1.data3,data1.data4,data1.sum_crc);

    // sBSP_UART_Debug_Printf("TYPE: 0x%02X", data1.type);
    // sBSP_UART_Debug_Printf(" DATA1: %.2f", (float)data1.data1 / 32768 * 16 * 9.81);
    // sBSP_UART_Debug_Printf(" DATA2: %.2f", (float)data1.data2 / 32768 * 16 * 9.81);
    // sBSP_UART_Debug_Printf(" DATA3: %.2f", (float)data1.data3 / 32768 * 16 * 9.81);
    // sBSP_UART_Debug_Printf(" DATA4: %.2f", (float)data1.data4 / 100);
    // sBSP_UART_Debug_Printf(" SUMCRC: 0x%02X\n", data1.sum_crc);

    //sBSP_UART_Debug_SendBytes((uint8_t*)pReciData,length);

END:
    sBSP_UART_IMU_RecvBegin(sDRV_JY901S_Handler);

}



