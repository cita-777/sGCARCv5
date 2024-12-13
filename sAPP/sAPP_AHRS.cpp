#include "sAPP_AHRS.hpp"


/**
 * sAPP_AHRS.cpp
 * 用于姿态估计
 * 
 */




#include "main.h"


AHRS ahrs;


AHRS::AHRS(){
    memset(&imu_sbias, 0, sizeof(IMU_StaticBias));
}


int AHRS::init(){
    get_data_mutex = xSemaphoreCreateMutex();
    
    #ifdef AHRS_IMU_SOURCE_WIT
        sDRV_JY901S_Init();
        HAL_NVIC_SetPriority(USART3_IRQn,4,0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);

    #endif
    #ifdef AHRS_IMU_SOURCE_9DOF
        this->icm42688  = &g_icm;
        this->icm45686 = &g_icm45686;
        this->lis3 = &g_lis3;

        /*把IMU的2个CS都上拉*/
        __GPIOC_CLK_ENABLE();
        GPIO_InitTypeDef gpio = {0};
        gpio.Mode  = GPIO_MODE_OUTPUT_PP;
        gpio.Pull  = GPIO_PULLUP;
        gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
        gpio.Pin   = ICM_CS_Pin | LIS3_CS_Pin;
        HAL_GPIO_Init(GPIOC,&gpio);
        HAL_GPIO_WritePin(GPIOC,ICM_CS_Pin ,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC,LIS3_CS_Pin,GPIO_PIN_SET);

        /*SPI2 <-> IMU_SPI*/
        sBSP_SPI_IMU_Init(SPI_BAUDRATEPRESCALER_4);   //11.25MBits/s

        #ifdef AHRS_IMU_USE_ICM42688
        if(sDRV_ICM_Init() != 0){
            return -1;
        }
        #endif
        #ifdef AHRS_IMU_USE_ICM45686
            if(sDRV_ICM45686_Init() != 0){
                return -1;
            }
        #endif
        if(sDRV_LIS3_Init() != 0){
            return -2;
        }

    #endif
    
    return 0;
}




int AHRS::calcBias(){
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
        update();
		acc_x_accu  += acc_x;
		acc_y_accu  += acc_y;
		acc_z_accu  += acc_z;
		gyro_x_accu += gyr_x;
		gyro_y_accu += gyr_y;
		gyro_z_accu += gyr_z;
        dwt.delay_us(10);
		// HAL_Delay(1);
	}
	imu_sbias.acc_x  = acc_x_accu  / POINT_COUNT;
	imu_sbias.acc_y  = acc_y_accu  / POINT_COUNT;
	imu_sbias.acc_z  = acc_z_accu  / POINT_COUNT - 9.81398f;	//重力加速度
	imu_sbias.gyr_x  = gyro_x_accu / POINT_COUNT;
	imu_sbias.gyr_y  = gyro_y_accu / POINT_COUNT;
	imu_sbias.gyr_z  = gyro_z_accu / POINT_COUNT;

    return 0;
}




int AHRS::update(){
    if(xSemaphoreTake(get_data_mutex,0) == pdTRUE){
        #ifdef AHRS_IMU_SOURCE_WIT
            sDRV_JY901S_Handler();
            imu.acc_x = g_jy901s.acc_x;
            imu.acc_y = g_jy901s.acc_y;
            imu.acc_z = g_jy901s.acc_z;
            imu.gyr_x = g_jy901s.gyr_x;
            imu.gyr_y = g_jy901s.gyr_y;
            imu.gyr_z = g_jy901s.gyr_z;
            imu.mag_x = g_jy901s.mag_x;
            imu.mag_y = g_jy901s.mag_y;
            imu.mag_z = g_jy901s.mag_z;
            ahrs.pitch = g_jy901s.pitch;
            ahrs.roll  = g_jy901s.roll;
            ahrs.yaw   = g_jy901s.yaw;
            ahrs.q0    = g_jy901s.q0;
            ahrs.q1    = g_jy901s.q1;
            ahrs.q2    = g_jy901s.q2;
            ahrs.q3    = g_jy901s.q3;
        #endif
        #ifdef AHRS_IMU_SOURCE_9DOF

            #ifdef AHRS_IMU_DATA_PERIOD_GET
                #ifdef AHRS_IMU_USE_ICM42688
                    sDRV_ICM_GetData();
                #endif
                #ifdef AHRS_IMU_USE_ICM45686
                    sDRV_ICM45686_GetData();
                    //减掉偏置
                    input.acc_x  = g_icm45686.acc_x  - imu_sbias.acc_x;
                    input.acc_y  = g_icm45686.acc_y  - imu_sbias.acc_y;
                    input.acc_z  = g_icm45686.acc_z  - imu_sbias.acc_z;
                    input.gyro_x = g_icm45686.gyro_x - imu_sbias.gyr_x;
                    input.gyro_y = g_icm45686.gyro_y - imu_sbias.gyr_y;
                    input.gyro_z = g_icm45686.gyro_z - imu_sbias.gyr_z;
                #endif
                sDRV_LIS3_GetData();
                mag_x = g_lis3.mag_x;
                mag_y = g_lis3.mag_y;
                mag_z = g_lis3.mag_z;
            #endif

            

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
            acc_x = input.acc_x;
            acc_y = input.acc_y;
            acc_z = input.acc_z;
            gyr_x = input.gyro_x;
            gyr_y = input.gyro_y;
            gyr_z = input.gyro_z;
            

            pitch = result.pitch;
            roll = result.roll;
            yaw = result.yaw;
            q0 = result.q0;
            q1 = result.q1;
            q2 = result.q2;
            q3 = result.q3;
        #endif
        
        //sBSP_UART_Debug_Printf("pitch: %6.2f, roll: %6.2f, yaw: %6.2f\n",pitch,roll,yaw);
        xSemaphoreGive(ahrs.get_data_mutex);
    }else{
        sDBG_Debug_Warning("AHRS::update获取get_data_mutex超时");
        return -1;
    }

    return 0;
}








