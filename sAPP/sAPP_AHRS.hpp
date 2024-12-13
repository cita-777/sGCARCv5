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


//选择数据来源,默认选择维特智能的IMU,取消这个宏将会选择ICM+LIS3惯导(互补滤波姿态解算)
//#define AHRS_IMU_SOURCE_WIT
#define AHRS_IMU_SOURCE_9DOF

#define AHRS_IMU_DATA_PERIOD_GET
// #define AHRS_IMU_DATA_INT_GET
// #define AHRS_IMU_USE_ICM42688
#define AHRS_IMU_USE_ICM45686



class AHRS{
public:
    AHRS();

    struct IMU_Data{     //IMU数据
        float acc_x,acc_y,acc_z;
        float gyr_x,gyr_y,gyr_z;
        float mag_x,mag_y,mag_z;
        float icm_temp,lis3_temp,tick;  //这个tick是ICM的tick
    };
    struct IMU_StaticBias{
        float acc_x,acc_y,acc_z;
        float gyr_x,gyr_y,gyr_z;
    };

    IMU_Data imu;
    IMU_StaticBias imu_sbias;
    
    SemaphoreHandle_t get_data_mutex;
    float acc_x,acc_y,acc_z;
    float gyr_x,gyr_y,gyr_z;
    float mag_x,mag_y,mag_z;
    float pitch,roll,yaw;
    float q0,q1,q2,q3;
    float icm_temp,lis3_temp;
    


    int init();
    int calcBias();
    int update();

private:
    sDRV_ICM_Data_t*  icm42688;
    sDRV_ICM45686_Data_t*  icm45686;
    sDRV_LIS3_Data_t* lis3;

    sLIB_6AXIS_INPUT_t input;
    sLIB_ATTITUDE_RESULT_t result;


};


extern AHRS ahrs;





