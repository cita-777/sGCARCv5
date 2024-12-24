#pragma once


#include "arm_math.h"
#include "math.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "sLib.h"


#include "sDRV_MB85RCxx.h"

#include "sDRV_ICM42688.h"
#include "sDRV_ICM45686.h"

#include "sDRV_LIS3MDLTR.h"
#include "sDRV_JY901S.h"


/*选择数据来源,默认选择维特智能的IMU,取消这个宏将会选择ICM+LIS3惯导(互补滤波姿态解算)*/
//#define AHRS_IMU_SOURCE_WIT
#define AHRS_IMU_SOURCE_9DOF
/*选择9DOF模块获取数据的方式,取消这个注释将使用period read方式读取数据*/
#define AHRS_IMU_ICM_INT_GET
/*选择9DOF模块所使用的IMU芯片*/
// #define AHRS_IMU_USE_ICM42688
#define AHRS_IMU_USE_ICM45686

//我的9DOF的IMU使用的是ICM+LIS3中断式数据更新模式,以降低读取latency


void sAPP_AHRS_Task(void* param);

extern"C" void sAPP_AHRS_ICMDataReadyCbISR();


class AHRS{
public:
    AHRS();

    //来自IMU的数据
    struct IMU_Data{
        float acc_x,acc_y,acc_z;
        float gyr_x,gyr_y,gyr_z;
        float mag_x,mag_y,mag_z;
        float icm_temp,lis3_temp;
    };
    //IMU静态零偏数据
    struct IMU_StaticBias{
        float acc_x,acc_y,acc_z;
        float gyr_x,gyr_y,gyr_z;
    };

    IMU_Data imu;
    IMU_StaticBias imu_sbias;
    
    typedef struct{
        float acc_x,acc_y,acc_z;
        float gyr_x,gyr_y,gyr_z;
        float mag_x,mag_y,mag_z;
        float pitch,roll,yaw;
        float q0,q1,q2,q3;
        
    }AHRS_Data;

    float icm_temp,lis3_temp;

    AHRS_Data dat;
    
    

    SemaphoreHandle_t mutex;


    int init();
    int calcBias();
    void get_imu_data();

    sLIB_6AXIS_INPUT_t input;
    sLIB_ATTITUDE_RESULT_t result;

private:
    sDRV_ICM_Data_t*  icm42688;
    sDRV_ICM45686_Data_t*  icm45686;
    sDRV_LIS3_Data_t* lis3;

    


};


extern AHRS ahrs;





