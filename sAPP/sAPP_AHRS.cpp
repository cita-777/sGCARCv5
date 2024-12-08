#include "sAPP_AHRS.hpp"


/**
 * sAPP_AHRS.cpp
 * 用于姿态估计
 * 
 */

//选择数据来源,默认选择维特智能的IMU,取消这个宏将会选择ICM+LIS3惯导(互补滤波姿态解算)
//#define USE_DATASOURCE_WITIMU


#include "main.h"

sAPP_AHRS ahrs;



int sAPP_AHRS::init(){
    #ifdef USE_DATASOURCE_WITIMU
        sDRV_JY901S_Init();
        HAL_NVIC_SetPriority(USART3_IRQn,4,0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);

    #else
        if(imu.init() != 0){
            //while(1);
        }
    #endif
    
    return 0;
}




int sAPP_AHRS::calcBias(){
    #define POINT_COUNT 30000


    HAL_Delay(1000);
    

	float acc_x_accu = 0;
	float acc_y_accu = 0;
	float acc_z_accu = 0;
	float gyro_x_accu = 0;
	float gyro_y_accu = 0;
	float gyro_z_accu = 0;
	for(uint16_t i = 0; i < POINT_COUNT; i++){
		//更新IMU采集的数据
        imu.update();
		acc_x_accu  += imu.acc_x;
		acc_y_accu  += imu.acc_y;
		acc_z_accu  += imu.acc_z;
		gyro_x_accu += imu.gyr_x;
		gyro_y_accu += imu.gyr_y;
		gyro_z_accu += imu.gyr_z;
        dwt.delay_us(10);
		// HAL_Delay(1);
	}
	bias_acc_x  = acc_x_accu  / POINT_COUNT;
	bias_acc_y  = acc_y_accu  / POINT_COUNT;
	bias_acc_z  = acc_z_accu  / POINT_COUNT - 9.81398f;	//重力加速度
	bias_gyro_x = gyro_x_accu / POINT_COUNT;
	bias_gyro_y = gyro_y_accu / POINT_COUNT;
	bias_gyro_z = gyro_z_accu / POINT_COUNT;

    return 0;
}


sLIB_6AXIS_INPUT_t input;
sLIB_ATTITUDE_RESULT_t result;

int sAPP_AHRS::update(){
    #ifdef USE_DATASOURCE_WITIMU
        sDRV_JY901S_Handler();
        ahrs.acc_x = g_jy901s.acc_x;
        ahrs.acc_y = g_jy901s.acc_y;
        ahrs.acc_z = g_jy901s.acc_z;
        ahrs.gyr_x = g_jy901s.gyr_x;
        ahrs.gyr_y = g_jy901s.gyr_y;
        ahrs.gyr_z = g_jy901s.gyr_z;
        ahrs.mag_x = g_jy901s.mag_x;
        ahrs.mag_y = g_jy901s.mag_y;
        ahrs.mag_z = g_jy901s.mag_z;
        ahrs.pitch = g_jy901s.pitch;
        ahrs.roll  = g_jy901s.roll;
        ahrs.yaw   = g_jy901s.yaw;
        ahrs.q0    = g_jy901s.q0;
        ahrs.q1    = g_jy901s.q1;
        ahrs.q2    = g_jy901s.q2;
        ahrs.q3    = g_jy901s.q3;
    #else
        imu.update();


        input.acc_x = imu.acc_x - bias_acc_x;
        input.acc_y = imu.acc_y - bias_acc_y;
        input.acc_z = imu.acc_z - bias_acc_z;
        input.gyro_x = imu.gyr_x - bias_gyro_x;
        input.gyro_y = imu.gyr_y - bias_gyro_y;
        input.gyro_z = imu.gyr_z - bias_gyro_z;

        // if(input.gyro_x < 0.1 && input.gyro_z > -0.1){
        //     input.gyro_x = 0;
        // }
        // if(input.gyro_y < 0.1 && input.gyro_y > -0.1){
        //     input.gyro_y = 0;
        // }
        // if(input.gyro_z < 0.1 && input.gyro_z > -0.1){
        //     input.gyro_z = 0;
        // }

        //融合算法
        sLib_6AxisCompFilter(&input, &result);
        ahrs.acc_x = input.acc_x;
        ahrs.acc_y = input.acc_y;
        ahrs.acc_z = input.acc_z;
        ahrs.gyr_x = input.gyro_x;
        ahrs.gyr_y = input.gyro_y;
        ahrs.gyr_z = input.gyro_z;

        ahrs.mag_x = g_lis3.mag_x;
        ahrs.mag_y = g_lis3.mag_y;
        ahrs.mag_z = g_lis3.mag_z;

        ahrs.pitch = result.pitch;
        ahrs.roll = result.roll;
        ahrs.yaw = result.yaw;
        ahrs.q0 = result.q0;
        ahrs.q1 = result.q1;
        ahrs.q2 = result.q2;
        ahrs.q3 = result.q3;
    #endif

    

    // sBSP_UART_Debug_Printf("pitch: %6.2f, roll: %6.2f, yaw: %6.2f\n",ahrs.pitch,ahrs.roll,ahrs.yaw);

    return 0;
}








