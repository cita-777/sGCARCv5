#include "sDRV_JY901S.h"

/**
 * sDRV_JY901S.c
 * 维特智能的JY901S IMU驱动
 * 使用UART通信
 * 参考:https://wit-motion.yuque.com/wumwnr/ltst03/vl3tpy?#S6al6
 * 
 * 241122 v1.0
 * 第一版 bySightseer. inHNIP 9607Lab
 * 
 */



//用于解包,类似0xFF 0xAA开头的
typedef struct{
    uint8_t type;    //TYPE
    int16_t data1;   //DATA1
    int16_t data2;   //DATA2
    int16_t data3;   //DATA3
    int16_t data4;   //DATA4
    uint8_t sum_crc; //校验和SUMCRC
}pkg_t;

typedef struct{
    pkg_t acc;  //加速度 m/^2
    pkg_t gyr;  //角速度 deg/s
    pkg_t deg;  //姿态角 deg
    pkg_t mag;  //磁力计 mGa? 单位不确定
    pkg_t qua;  //四元数 
}pkg_set_t;

static pkg_set_t pkg;

//接收锁
static bool recv_lock = false;
//新数据已准备好
static bool new_data_ready = false;

//接收缓冲区
static uint8_t recv_data_buf[55];


sDRV_JY901S_t g_jy901s;



/*******************************************接口*******************************************/
static void portUARTInit(uint32_t bandrate){
    sBSP_UART_IMU_Init(bandrate);
}
static inline void portSendByte(uint8_t data){
    sBSP_UART_IMU_SendByte(data);
}
//串口启动一次接收
static inline void portRecvBegin(){
    sBSP_UART_IMU_RecvBegin(sDRV_JY901S_RecvDataCb);
}

//用于串口发送数据
static void sendBytes(const char* pData,uint8_t len){
    //sBSP_UART_Debug_SendBytes((uint8_t*)pData,len);
    sBSP_UART_IMU_SendBytes((uint8_t*)pData,len);
    // sBSP_UART_Debug_Printf("Send:0x%02X 0x%02X 0x%02X 0x%02X 0x%02X Len:%u \n"\
    // ,pData[0],pData[1],pData[2],pData[3],pData[4],len);
}

//前面无下划线:无参宏 有一个下划线:带参宏,有两个:功能性宏,有三个:用于中间操作,用户不要调用

//SAVE 保存/重启/恢复出厂 RW
#define ADDR_SAVE           "\x00"
#define SAVE_SAVE           "\x00\x00"
#define SAVE_REBOOT         "\xFF\x00"
#define SAVE_RESTORE        "\x01\x00"

//KEY 解锁
#define ADDR_KEY            "\x69"
#define KEY_KEY             "\x88\xB5" //写0xB588解锁

//CALSW 校准模式 RW ADDR:0x01 
#define ADDR_CALSW          "\x01"
#define CALSW_NORMAL        "\x00\x00" //正常工作模式
#define CALSW_AUTOCAL       "\x01\x00" //自动加计校准
#define CALSW_ALTCLR        "\x03\x00" //高度清零
#define CALSW_HEADINGCLR    "\x04\x00" //航向角置0
#define CALSW_MAGCAL1       "\x07\x00" //磁场校准(球形拟合法)
#define CALSW_SETDEGREF     "\x08\x00" //设置角度参考
#define CALSW_MAGCAL2       "\x09\x00" //磁场校准(双平面模式)

//RSW 输出内容 RW ADDR:0x02
#define ADDR_RSW            "\x02"
#define RSW_IMU             "\x1E\x02" //设置输出内容为:加速度 角速度 角度 磁场 四元数
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


//设置波特率115200
//注意,这里没有用到设置波特率,因为之前波特率和现在不一样的话,发过去IMU没办法解包,所以要先在上位机上设置好波特率
#define COMM_BANDRATE_115200 "\xFF\xAA\x04\x06\x00"



//构造命令
#define _COMM(__ADDR__, __VAL__)        "\xFF\xAA"__ADDR__ __VAL__
//获取指令长度, -1是为了跳过\n
#define __COMM_LEN(__COMM__)            (sizeof(__COMM__) - 1)

//调用这个就可以发送命令
#define __SEND_COMM_UNLOCK(__X__)       sendBytes(_COMM(ADDR_KEY,KEY_KEY),__COMM_LEN(_COMM(ADDR_KEY,KEY_KEY)));
#define __SEND_COMM_SAVE(__X__)         sendBytes(_COMM(ADDR_SAVE,SAVE_SAVE),__COMM_LEN(_COMM(ADDR_SAVE,SAVE_SAVE)));

#define __SEND_COMM_DEGREF(__X__)       sendBytes(_COMM(ADDR_CALSW,CALSW_SETDEGREF),__COMM_LEN(_COMM(ADDR_CALSW,CALSW_SETDEGREF)));

#define __SEND_COMM_RRATE10HZ(__X__)    sendBytes(_COMM(ADDR_RRATE,RRATE_10HZ),__COMM_LEN(_COMM(ADDR_RRATE,RRATE_10HZ)));
#define __SEND_COMM_RRATE100HZ(__X__)   sendBytes(_COMM(ADDR_RRATE,RRATE_100HZ),__COMM_LEN(_COMM(ADDR_RRATE,RRATE_100HZ)));
#define __SEND_COMM_RRATENEVER(__X__)   sendBytes(_COMM(ADDR_RRATE,RRATE_NEVER),__COMM_LEN(_COMM(ADDR_RRATE,RRATE_NEVER)));


#define __SEND_COMM_CONF_IMU(__X__)     sendBytes(_COMM(ADDR_RSW,RSW_IMU),__COMM_LEN(_COMM(ADDR_RSW,RSW_IMU)));

#define __SEND_COMM_AXIS6(__X__)        sendBytes(_COMM(ADDR_AXIS6,AXIS6_6AXIS),__COMM_LEN(_COMM(ADDR_AXIS6,AXIS6_6AXIS)));
#define __SEND_COMM_AXIS9(__X__)        sendBytes(_COMM(ADDR_AXIS6,AXIS6_9AXIS),__COMM_LEN(_COMM(ADDR_AXIS6,AXIS6_9AXIS)));



int sDRV_JY901S_Init(){
    //注意先在上位机上配置好波特率115200
    portUARTInit(115200);

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

    // sDRV_JY901S_SetRRATE100Hz();

    //开始接收数据
    portRecvBegin();

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

void sDRV_JY901S_SetRRATE100Hz(){
    //解锁
    __SEND_COMM_UNLOCK();
    //等待200ms
    HAL_Delay(200);
    //配置输出
    __SEND_COMM_RRATE100HZ();
    //等待200ms
    HAL_Delay(200);
    //保存
    __SEND_COMM_SAVE();
}

//设置角度参考,发送这个就可以让XY轴归零,以当前平面作为参考
void sDRV_JY901S_SetDegRef(){
    //解锁
    __SEND_COMM_UNLOCK();
    //等待200ms
    HAL_Delay(200);
    __SEND_COMM_DEGREF();
    //等待200ms
    HAL_Delay(200);
    //保存
    __SEND_COMM_SAVE();
    
}


//接收数据回调,串口接收完调用(可以由中断调用),需要提供指向接收缓冲区的指针和数据包长度
void sDRV_JY901S_RecvDataCb(char* pRecvData,uint16_t length){
    //如果数据包长度不对则丢弃
    if(length != 55){goto END;}
    //加锁
    if(recv_lock == true){return;}else{recv_lock = true;}
    //复制缓冲区数据
    memcpy((void*)recv_data_buf,(const char*)pRecvData,(size_t)55);
    //表明新数据准备好了
    new_data_ready = true;

END:
    //解锁
    recv_lock = false;
    //重新启动一次接收
    portRecvBegin();
}


//处理函数,调用一次计算并更新一次数据
void sDRV_JY901S_Handler(){
    //如果还没有新数据来就不处理
    if(new_data_ready == false){return;}else{new_data_ready = false;}

    /*把接收到的5个数据包变成pkg_set_t的格式,方便后续处理,关于为什么这么做可以参考维特智能数据包格式文档*/
    //单包长度
    #define DATA_PKG_LEN 11
    //指向当前包
    pkg_t* now_pkg = &pkg.acc;
    //指向当前数据字段
    int16_t* now_data = NULL;
    //复制数据
    for(int pkg_cnt = 0; pkg_cnt < 5; pkg_cnt++){ //5个数据包
        //复制类型字段
        now_pkg[pkg_cnt].type = recv_data_buf[1 + pkg_cnt * DATA_PKG_LEN];
        //让now_data指向data1字段
        now_data = &now_pkg[pkg_cnt].data1;
        //每个data字段的偏移量
        uint16_t offset = pkg_cnt * DATA_PKG_LEN + sizeof(int16_t);
        for(int data_cnt = 0; data_cnt < 4; data_cnt++){ //复制4个data字段
            now_data[data_cnt] = (recv_data_buf[offset + 1] << 8) | recv_data_buf[offset + 0];
            offset += sizeof(int16_t);
        }
        //复制sum_crc字段
        now_pkg[pkg_cnt].sum_crc = recv_data_buf[10 + pkg_cnt * DATA_PKG_LEN];
    }


    /*转换为标准单位*/
    //m/s^2
    g_jy901s.acc_x = (float)pkg.acc.data1 / 32768.0f * 16.0f * 9.81f;
    g_jy901s.acc_y = (float)pkg.acc.data2 / 32768.0f * 16.0f * 9.81f;
    g_jy901s.acc_z = (float)pkg.acc.data3 / 32768.0f * 16.0f * 9.81f;
    g_jy901s.temp  = (float)pkg.acc.data4 / 100.0f;
    //deg
    g_jy901s.gyr_x = (float)pkg.gyr.data1 / 32768.0f * 2000.0f;
    g_jy901s.gyr_y = (float)pkg.gyr.data2 / 32768.0f * 2000.0f;
    g_jy901s.gyr_z = (float)pkg.gyr.data3 / 32768.0f * 2000.0f;
    //deg 这里滚转角和俯仰角互换了
    g_jy901s.pitch = (float)pkg.deg.data1 / 32768.0f * 180.0f;
    g_jy901s.roll  = (float)pkg.deg.data2 / 32768.0f * 180.0f;
    g_jy901s.yaw   = (float)pkg.deg.data3 / 32768.0f * 180.0f;
    //mag 这个单位好像是LSB,维特智能没提供转换到mGa/uT的方法
    g_jy901s.mag_x = (float)pkg.mag.data1;
    g_jy901s.mag_y = (float)pkg.mag.data2;
    g_jy901s.mag_z = (float)pkg.mag.data3;
    //quaternion
    g_jy901s.q0    = (float)pkg.qua.data1;
    g_jy901s.q1    = (float)pkg.qua.data2;
    g_jy901s.q2    = (float)pkg.qua.data3;
    g_jy901s.q3    = (float)pkg.qua.data4;



    // sBSP_UART_Debug_Printf("ax:%.2f,ay:%.2f,az:%.2f,gx:%.2f,gy:%.2f,gz:%.2f,p:%.2f,r:%.2f,y:%.2f,mx:%.2f,my:%.2f,mz:%.2f\n",\
    // g_jy901s.acc_x,g_jy901s.acc_y,g_jy901s.acc_z,\
    // g_jy901s.gyr_x,g_jy901s.gyr_y,g_jy901s.gyr_z,\
    // g_jy901s.pitch,g_jy901s.roll ,g_jy901s.yaw,\
    // g_jy901s.mag_x,g_jy901s.mag_y,g_jy901s.mag_z);


    // sBSP_UART_Debug_Printf("TYPE: 0x%02X,%d,%d,%d,%d,%d\n",\
    // data1.type,data1.data1,data1.data2,data1.data3,data1.data4,data1.sum_crc);

    // sBSP_UART_Debug_Printf("TYPE: 0x%02X", data1.type);
    // sBSP_UART_Debug_Printf(" DATA1: %.2f", (float)data1.data1 / 32768 * 16 * 9.81);
    // sBSP_UART_Debug_Printf(" DATA2: %.2f", (float)data1.data2 / 32768 * 16 * 9.81);
    // sBSP_UART_Debug_Printf(" DATA3: %.2f", (float)data1.data3 / 32768 * 16 * 9.81);
    // sBSP_UART_Debug_Printf(" DATA4: %.2f", (float)data1.data4 / 100);
    // sBSP_UART_Debug_Printf(" SUMCRC: 0x%02X\n", data1.sum_crc);

    //sBSP_UART_Debug_SendBytes((uint8_t*)pReciData,length);

}



