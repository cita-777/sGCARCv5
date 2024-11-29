#pragma once


#include "sDRV_JY901S.h"


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


    int init();
    int calcBias();
    int update();

private:

};


extern sAPP_AHRS ahrs;





