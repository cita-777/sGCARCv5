#pragma once


#include "sDRV_JY901S.h"
#include "sDRV_MB85RCxx.h"

class sAPP_AHRS{
public:
    float gyr_x;
    float gyr_y;
    float gyr_z;
    float acc_x;
    float acc_y;
    float acc_z;
    float mag_x;
    float mag_y;
    float mag_z;


    float pitch;
    float roll;
    float yaw;

    float q0,q1,q2,q3;

    float bias_acc_x ;
    float bias_acc_y ;
    float bias_acc_z ;
    float bias_gyro_x;
    float bias_gyro_y;
    float bias_gyro_z;


    int init();
    int calcBias();
    int update();

private:

};


extern sAPP_AHRS ahrs;





