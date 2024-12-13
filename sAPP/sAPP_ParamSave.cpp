#include "sAPP_ParamSave.hpp"



#define ADDR_IMU_BIAS            (0x0010u)
typedef struct __packed{
    float acc_x,acc_y,acc_z;
    float gyr_x,gyr_y,gyr_z;
    bool is_calibrated;
}imu_bias_t;
static imu_bias_t imu_bias;
#define SIZEOF_IMU_BIAS          (sizeof(imu_bias_t))





static inline uint8_t portReadByte(uint16_t addr){
    return sDRV_MB85RCxx_ReadByte(addr);
}

static inline void portReadBytes(uint16_t addr,void* pData,uint16_t len){
    sDRV_MB85RCxx_ReadBytes(addr,(uint8_t*)pData,len);
}

static inline uint8_t portWriteByte(uint16_t addr,uint8_t byte){
    return sDRV_MB85RCxx_WriteByte(addr,byte);
}

static inline void portWriteBytes(uint16_t addr,void* pData,uint16_t len){
    sDRV_MB85RCxx_WriteBytes(addr,(uint8_t*)pData,len);
}




void sAPP_ParamSave_Init(){
    sDRV_MB85RCxx_Init();
    sDRV_MB85RCxx_Format(0);
}



void sAPP_ParamSave_CheckIMUStaticBias(){

    portReadBytes(ADDR_IMU_BIAS,&imu_bias,sizeof(imu_bias));
    if(!imu_bias.is_calibrated){
        sBSP_UART_Debug_Printf("IMU还没有进行零偏校准,将在1s后进行校准...\n");
        ahrs.calcBias();
        imu_bias.acc_x = ahrs.imu_sbias.acc_x;
        imu_bias.acc_y = ahrs.imu_sbias.acc_y;
        imu_bias.acc_z = ahrs.imu_sbias.acc_z;
        imu_bias.gyr_x = ahrs.imu_sbias.gyr_x;
        imu_bias.gyr_y = ahrs.imu_sbias.gyr_y;
        imu_bias.gyr_z = ahrs.imu_sbias.gyr_z;
        imu_bias.is_calibrated = true;
        portWriteBytes(ADDR_IMU_BIAS,&imu_bias,sizeof(imu_bias));
        sBSP_UART_Debug_Printf("校准完成! 校准值:%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",imu_bias.acc_x,imu_bias.acc_y,
        imu_bias.acc_z,imu_bias.gyr_x,imu_bias.gyr_y,imu_bias.gyr_z);
    }
    else{
        sBSP_UART_Debug_Printf("检测到IMU已经校准! 读取的校准值:");
        sBSP_UART_Debug_Printf("%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",imu_bias.acc_x,imu_bias.acc_y,
        imu_bias.acc_z,imu_bias.gyr_x,imu_bias.gyr_y,imu_bias.gyr_z);
        ahrs.imu_sbias.acc_x = imu_bias.acc_x;
        ahrs.imu_sbias.acc_y = imu_bias.acc_y;
        ahrs.imu_sbias.acc_z = imu_bias.acc_z;
        ahrs.imu_sbias.gyr_x = imu_bias.gyr_x;
        ahrs.imu_sbias.gyr_y = imu_bias.gyr_y;
        ahrs.imu_sbias.gyr_z = imu_bias.gyr_z;
        sBSP_UART_Debug_Printf("IMU校准值已应用!\n");
    }
}





