#include "sAPP_AHRS.hpp"


/**
 * sAPP_AHRS.cpp
 * 用于姿态估计
 * 
 * 
 * v1.1 241223 bySightseer.
 * ! 注意,因为改变了IMU数据获取方式,从直接获取变成了中断式获取,以降低latency
 * ! 所以,现在暂时不能用除ICM45686以外的其他IMU!!!
 * 
 * 
 */




#include "main.h"


AHRS ahrs;


//ICM数据就绪二值信号量
SemaphoreHandle_t icm_data_ready_bin;


AHRS::AHRS(){
    memset(&imu_sbias, 0, sizeof(IMU_StaticBias));
}


static bool flag = 1;

static void complementary_filter(sLIB_6AXIS_INPUT_t* input,sLIB_ATTITUDE_RESULT_t* result){
    static uint32_t prev_time;
    static uint32_t now_time;
    static float dts;
    prev_time = now_time;
    now_time = HAL_GetTick();
    dts = (now_time - prev_time) / 1000.0f;

    if(flag){
        dts = 0;
        flag = 0;
    }

    const float Kp = 0.05f;
	const float Ki = 0.001f;

    static float q0 = 1.0f;
	static float q1 = 0.0f;
	static float q2 = 0.0f;
	static float q3 = 0.0f;	

    //误差积分累计
	float exInt = 0.0f;
	float eyInt = 0.0f;
	float ezInt = 0.0f;

    //用于缓存数据,便于修改
    static sLIB_6AXIS_INPUT_t data;
    //拷贝数据
    memcpy(&data, input, sizeof(sLIB_6AXIS_INPUT_t));

    //把陀螺仪的角度单位换算成弧度
    data.gyro_x *= DEG2RAD;
	data.gyro_y *= DEG2RAD;
	data.gyro_z *= DEG2RAD;

	//标准化加速计测量值
	float acc_norm = sLib_InvSqrt(data.acc_x * data.acc_x + data.acc_y * data.acc_y + data.acc_z * data.acc_z);
	data.acc_x *= acc_norm;
	data.acc_y *= acc_norm;
	data.acc_z *= acc_norm;

    static float rMat[3][3];


	float a_vx = rMat[2][0];
	float a_vy = rMat[2][1];
	float a_vz = rMat[2][2];

    float a_ex = data.acc_y * a_vz - data.acc_z * a_vy;
    float a_ey = data.acc_z * a_vx - data.acc_x * a_vz;
    float a_ez = data.acc_x * a_vy - data.acc_y * a_vx;

    //误差累计，与积分常数相乘
	exInt += Ki * a_ex * dts;  
	eyInt += Ki * a_ey * dts;
	ezInt += Ki * a_ez * dts;

	//用叉积误差来做PI修正陀螺零偏，即抵消陀螺读数中的偏移量
	data.gyro_x += Kp * a_ex + exInt;
	data.gyro_y += Kp * a_ey + eyInt;
	data.gyro_z += Kp * a_ez + ezInt;

    //一阶近似算法，四元数运动学方程的离散化形式和积分
	float q0Last = q0;
	float q1Last = q1;
	float q2Last = q2;
	float q3Last = q3;
	q0 += (-q1Last * data.gyro_x - q2Last * data.gyro_y - q3Last * data.gyro_z) * 0.5f * dts;
	q1 += ( q0Last * data.gyro_x + q2Last * data.gyro_z - q3Last * data.gyro_y) * 0.5f * dts;
	q2 += ( q0Last * data.gyro_y - q1Last * data.gyro_z + q3Last * data.gyro_x) * 0.5f * dts;
	q3 += ( q0Last * data.gyro_z + q1Last * data.gyro_y - q2Last * data.gyro_x) * 0.5f * dts;

    //标准化四元数
	float q_norm = sLib_InvSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= q_norm;
	q1 *= q_norm;
	q2 *= q_norm;
	q3 *= q_norm;


    //计算方向余弦矩阵
    float q1q1 = q1 * q1;
    float q2q2 = q2 * q2;
    float q3q3 = q3 * q3;

    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q0q3 = q0 * q3;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q2q3 = q2 * q3;

    rMat[0][0] = 1.0f - 2.0f * q2q2 - 2.0f * q3q3;
    rMat[0][1] = 2.0f * (q1q2 + -q0q3);
    rMat[0][2] = 2.0f * (q1q3 - -q0q2);

    rMat[1][0] = 2.0f * (q1q2 - -q0q3);
    rMat[1][1] = 1.0f - 2.0f * q1q1 - 2.0f * q3q3;
    rMat[1][2] = 2.0f * (q2q3 + -q0q1);

    rMat[2][0] = 2.0f * (q1q3 - q0q2);
    rMat[2][1] = 2.0f * (q2q3 + q0q1);
    rMat[2][2] = 1.0f - 2.0f * q1q1 - 2.0f * q2q2;

    //计算roll pitch yaw 欧拉角
	result->roll  = -asinf(rMat[2][0]) * RAD2DEG; 
	result->pitch =  atan2f(rMat[2][1], rMat[2][2]) * RAD2DEG;
	result->yaw   =  atan2f(rMat[1][0], rMat[0][0]) * RAD2DEG;
    //保存四元数
    result->q0 = q0;
    result->q1 = q1;
    result->q2 = q2;
    result->q3 = q3;

}

// static void complementary_filter(sLIB_6AXIS_INPUT_t* input,sLIB_ATTITUDE_RESULT_t* result\
// ,float mag_x,float mag_y,float mag_z){
//     static uint32_t prev_time;
//     static uint32_t now_time;
//     static float dts;
//     prev_time = now_time;
//     now_time = HAL_GetTick();
//     dts = (now_time - prev_time) / 1000.0f;

//     if(flag){
//         dts = 0;
//         flag = 0;
//     }

//     const float Kp = 0.05f;
// 	const float Ki = 0.001f;

//     static float q0 = 1.0f;
// 	static float q1 = 0.0f;
// 	static float q2 = 0.0f;
// 	static float q3 = 0.0f;	

//     //误差积分累计
// 	float exInt = 0.0f;
// 	float eyInt = 0.0f;
// 	float ezInt = 0.0f;

//     //用于缓存数据,便于修改
//     static sLIB_6AXIS_INPUT_t data;
//     //拷贝数据
//     memcpy(&data, input, sizeof(sLIB_6AXIS_INPUT_t));

//     //把陀螺仪的角度单位换算成弧度
//     data.gyro_x *= DEG2RAD;
// 	data.gyro_y *= DEG2RAD;
// 	data.gyro_z *= DEG2RAD;

// 	//标准化加速计测量值
// 	float acc_norm = sLib_InvSqrt(data.acc_x * data.acc_x + data.acc_y * data.acc_y + data.acc_z * data.acc_z);
// 	data.acc_x *= acc_norm;
// 	data.acc_y *= acc_norm;
// 	data.acc_z *= acc_norm;

//     float mag_norm = sLib_InvSqrt(mag_x * mag_x + mag_y * mag_y + mag_z * mag_z);
//     mag_x *= mag_norm;
//     mag_y *= mag_norm;
//     mag_z *= mag_norm;


//     static float rMat[3][3];


// 	float a_vx = rMat[2][0];
// 	float a_vy = rMat[2][1];
// 	float a_vz = rMat[2][2];

//     float a_ex = data.acc_y * a_vz - data.acc_z * a_vy;
//     float a_ey = data.acc_z * a_vx - data.acc_x * a_vz;
//     float a_ez = data.acc_x * a_vy - data.acc_y * a_vx;

//     //误差累计，与积分常数相乘
// 	exInt += Ki * a_ex * dts;  
// 	eyInt += Ki * a_ey * dts;
// 	ezInt += Ki * a_ez * dts;

// 	//用叉积误差来做PI修正陀螺零偏，即抵消陀螺读数中的偏移量
// 	data.gyro_x += Kp * a_ex + exInt;
// 	data.gyro_y += Kp * a_ey + eyInt;
// 	data.gyro_z += Kp * a_ez + ezInt;

//     //一阶近似算法，四元数运动学方程的离散化形式和积分
// 	float q0Last = q0;
// 	float q1Last = q1;
// 	float q2Last = q2;
// 	float q3Last = q3;
// 	q0 += (-q1Last * data.gyro_x - q2Last * data.gyro_y - q3Last * data.gyro_z) * 0.5f * dts;
// 	q1 += ( q0Last * data.gyro_x + q2Last * data.gyro_z - q3Last * data.gyro_y) * 0.5f * dts;
// 	q2 += ( q0Last * data.gyro_y - q1Last * data.gyro_z + q3Last * data.gyro_x) * 0.5f * dts;
// 	q3 += ( q0Last * data.gyro_z + q1Last * data.gyro_y - q2Last * data.gyro_x) * 0.5f * dts;

//     //标准化四元数
// 	float q_norm = sLib_InvSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
// 	q0 *= q_norm;
// 	q1 *= q_norm;
// 	q2 *= q_norm;
// 	q3 *= q_norm;


//     //计算方向余弦矩阵
//     float q1q1 = q1 * q1;
//     float q2q2 = q2 * q2;
//     float q3q3 = q3 * q3;

//     float q0q1 = q0 * q1;
//     float q0q2 = q0 * q2;
//     float q0q3 = q0 * q3;
//     float q1q2 = q1 * q2;
//     float q1q3 = q1 * q3;
//     float q2q3 = q2 * q3;

//     rMat[0][0] = 1.0f - 2.0f * q2q2 - 2.0f * q3q3;
//     rMat[0][1] = 2.0f * (q1q2 + -q0q3);
//     rMat[0][2] = 2.0f * (q1q3 - -q0q2);

//     rMat[1][0] = 2.0f * (q1q2 - -q0q3);
//     rMat[1][1] = 1.0f - 2.0f * q1q1 - 2.0f * q3q3;
//     rMat[1][2] = 2.0f * (q2q3 + -q0q1);

//     rMat[2][0] = 2.0f * (q1q3 - q0q2);
//     rMat[2][1] = 2.0f * (q2q3 + q0q1);
//     rMat[2][2] = 1.0f - 2.0f * q1q1 - 2.0f * q2q2;

//     //计算roll pitch yaw 欧拉角
// 	result->roll  = -asinf(rMat[2][0]) * RAD2DEG; 
// 	result->pitch =  atan2f(rMat[2][1], rMat[2][2]) * RAD2DEG;
// 	result->yaw   =  atan2f(rMat[1][0], rMat[0][0]) * RAD2DEG;
//     //保存四元数
//     result->q0 = q0;
//     result->q1 = q1;
//     result->q2 = q2;
//     result->q3 = q3;

// }




int AHRS::init(){
    //创建ICM数据就绪的二值信号量
    icm_data_ready_bin = xSemaphoreCreateBinary();
    //创建AHRS数据互斥锁
    mutex = xSemaphoreCreateMutex();
    
    #ifdef AHRS_IMU_SOURCE_WIT
        sDRV_JY901S_Init();
        HAL_NVIC_SetPriority(USART3_IRQn,4,0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
    #endif
    #ifdef AHRS_IMU_SOURCE_9DOF
        this->icm42688  = &g_icm;
        this->icm45686  = &g_icm45686;
        this->lis3      = &g_lis3;

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
                sBSP_UART_Debug_Printf("[ERR ]ICM45688初始化失败\n");
                return -1;
            }
        #endif
        if(sDRV_LIS3_Init() != 0){
            sBSP_UART_Debug_Printf("[ERR ]LIS3MDLTR初始化失败\n");
            return -2;
        }
        
        

    #endif
    
    return 0;
}


int AHRS::calcBias(){
    #define POINT_COUNT 3000
    // HAL_Delay(1000);
    vTaskDelay(1000);
	float acc_x_accu = 0;
	float acc_y_accu = 0;
	float acc_z_accu = 0;
	float gyro_x_accu = 0;
	float gyro_y_accu = 0;
	float gyro_z_accu = 0;
	for(uint16_t i = 0; i < POINT_COUNT; i++){
        //减掉偏置是为了读到原始数据
		acc_x_accu  += dat.acc_x + imu_sbias.acc_x;
		acc_y_accu  += dat.acc_y + imu_sbias.acc_y;
		acc_z_accu  += dat.acc_z + imu_sbias.acc_z;
		gyro_x_accu += dat.gyr_x + imu_sbias.gyr_x;
		gyro_y_accu += dat.gyr_y + imu_sbias.gyr_y;
		gyro_z_accu += dat.gyr_z + imu_sbias.gyr_z;
        // dwt.delay_us(10);
		// HAL_Delay(1);
        vTaskDelay(1);
	}
	imu_sbias.acc_x  = acc_x_accu  / POINT_COUNT;
	imu_sbias.acc_y  = acc_y_accu  / POINT_COUNT;
	imu_sbias.acc_z  = acc_z_accu  / POINT_COUNT - 9.81398f;	//重力加速度
	imu_sbias.gyr_x  = gyro_x_accu / POINT_COUNT;
	imu_sbias.gyr_y  = gyro_y_accu / POINT_COUNT;
	imu_sbias.gyr_z  = gyro_z_accu / POINT_COUNT;

    return 0;
}



void AHRS::get_imu_data(){
    #ifdef AHRS_IMU_SOURCE_WIT
        sDRV_JY901S_Handler();
        // imu.acc_x = g_jy901s.acc_x;
        // imu.acc_y = g_jy901s.acc_y;
        // imu.acc_z = g_jy901s.acc_z;
        // imu.gyr_x = g_jy901s.gyr_x;
        // imu.gyr_y = g_jy901s.gyr_y;
        // imu.gyr_z = g_jy901s.gyr_z;
        // imu.mag_x = g_jy901s.mag_x;
        // imu.mag_y = g_jy901s.mag_y;
        // imu.mag_z = g_jy901s.mag_z;
        // ahrs.pitch = g_jy901s.pitch;
        // ahrs.roll  = g_jy901s.roll;
        // ahrs.yaw   = g_jy901s.yaw;
        // ahrs.q0    = g_jy901s.q0;
        // ahrs.q1    = g_jy901s.q1;
        // ahrs.q2    = g_jy901s.q2;
        // ahrs.q3    = g_jy901s.q3;
    #endif
    #ifdef AHRS_IMU_SOURCE_9DOF
        #ifdef AHRS_IMU_USE_ICM42688
            sDRV_ICM_GetData();
            //减掉偏置
            input.acc_x  = g_icm.acc_x  - imu_sbias.acc_x;
            input.acc_y  = g_icm.acc_y  - imu_sbias.acc_y;
            input.acc_z  = g_icm.acc_z  - imu_sbias.acc_z;
            input.gyro_x = g_icm.gyro_x - imu_sbias.gyr_x;
            input.gyro_y = g_icm.gyro_y - imu_sbias.gyr_y;
            input.gyro_z = g_icm.gyro_z - imu_sbias.gyr_z;
        #endif
        #ifdef AHRS_IMU_USE_ICM45686
            sDRV_ICM45686_GetData();
            //减掉偏置
            input.acc_x  = g_icm45686.acc_x - imu_sbias.acc_x;
            input.acc_y  = g_icm45686.acc_y - imu_sbias.acc_y;
            input.acc_z  = g_icm45686.acc_z - imu_sbias.acc_z;
            input.gyro_x = g_icm45686.gyr_x - imu_sbias.gyr_x;
            input.gyro_y = g_icm45686.gyr_y - imu_sbias.gyr_y;
            input.gyro_z = g_icm45686.gyr_z - imu_sbias.gyr_z;
        #endif
        sDRV_LIS3_GetData();
        dat.mag_x = g_lis3.mag_x;
        dat.mag_y = g_lis3.mag_y;
        dat.mag_z = g_lis3.mag_z;
    #endif
}

//AHRS任务,非阻塞式获取数据
void sAPP_AHRS_Task(void* param){
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    for(;;){
        //当数据准备就绪则运行AHRS算法,此步骤消耗时间60us
        if(xSemaphoreTake(icm_data_ready_bin,200) == pdTRUE){
            sDRV_JY901S_Handler();
            sDRV_ICM45686_GetData();
            //获取AHRS数据互斥锁
            if(xSemaphoreTake(ahrs.mutex,200) == pdTRUE){
                //减掉静态偏置,这里g_icm45686只在这个task里运行,所以不用加锁
                ahrs.dat.acc_x = g_icm45686.acc_x - ahrs.imu_sbias.acc_x;
                ahrs.dat.acc_y = g_icm45686.acc_y - ahrs.imu_sbias.acc_y;
                ahrs.dat.acc_z = g_icm45686.acc_z - ahrs.imu_sbias.acc_z;
                ahrs.dat.gyr_x = g_icm45686.gyr_x - ahrs.imu_sbias.gyr_x;
                ahrs.dat.gyr_y = g_icm45686.gyr_y - ahrs.imu_sbias.gyr_y;
                ahrs.dat.gyr_z = g_icm45686.gyr_z - ahrs.imu_sbias.gyr_z;
                ahrs.icm_temp  = g_icm45686.temp;

                //给互补滤波准备数据
                ahrs.input.acc_x  = ahrs.dat.acc_x; 
                ahrs.input.acc_y  = ahrs.dat.acc_y; 
                ahrs.input.acc_z  = ahrs.dat.acc_z; 
                ahrs.input.gyro_x = ahrs.dat.gyr_x;
                ahrs.input.gyro_y = ahrs.dat.gyr_y;
                ahrs.input.gyro_z = ahrs.dat.gyr_z;
                //融合算法
                // sLib_6AxisCompFilter(&ahrs.input, &ahrs.result);
                complementary_filter(&ahrs.input, &ahrs.result);
                ahrs.dat.pitch = ahrs.result.pitch;
                ahrs.dat.roll  = ahrs.result.roll;
                ahrs.dat.yaw   = ahrs.result.yaw;
                ahrs.dat.q0    = ahrs.result.q0;
                ahrs.dat.q1    = ahrs.result.q1;
                ahrs.dat.q2    = ahrs.result.q2;
                ahrs.dat.q3    = ahrs.result.q3;

                // sDRV_LIS3_GetData();
                // ahrs.dat.mag_x = g_lis3.mag_x;
                // ahrs.dat.mag_y = g_lis3.mag_y;
                // ahrs.dat.mag_z = g_lis3.mag_z;

                //把新获取到的数据通过队列发送给blc_ctrl算法
                // xQueueSend(g_blc_ctrl_ahrs_queue,&ahrs.dat,200);
                xQueueOverwrite(g_blc_ctrl_ahrs_queue,&ahrs.dat); 

                xSemaphoreGive(ahrs.mutex);
            }else{
                sBSP_UART_Debug_Printf("[ERR]AHRS错误: 获取ahrs.mutex超时\n");
                Error_Handler();
            }
            // sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f,",ahrs.dat.acc_x,ahrs.dat.acc_y,ahrs.dat.acc_z);
            // sBSP_UART_Debug_Printf("%6.2f,%6.2f,%6.2f,%u\n",ahrs.dat.gyr_x,ahrs.dat.gyr_y,ahrs.dat.gyr_z,HAL_GetTick());
            // sBSP_UART_Debug_Printf("%6.2f\n",ahrs.icm_temp);
            // sBSP_UART_Debug_Printf("pitch: %6.2f, roll: %6.2f, yaw: %6.2f\n",ahrs.pitch,ahrs.roll,ahrs.yaw);

            // sBSP_UART_Debug_Printf("%.2f,%.2f,%.2f,",ahrs.dat.acc_x,ahrs.dat.acc_y,ahrs.dat.acc_z);
            // sBSP_UART_Debug_Printf("%.2f,%.2f,%.2f,",ahrs.dat.gyr_x,ahrs.dat.gyr_y,ahrs.dat.gyr_z);
            // sBSP_UART_Debug_Printf("%.2f,%.2f,%.2f,",g_jy901s.pitch,g_jy901s.roll,g_jy901s.yaw);
            // sBSP_UART_Debug_Printf("%.2f,%.2f,%.2f,",ahrs.dat.pitch,ahrs.dat.roll,ahrs.dat.yaw);
            // sBSP_UART_Debug_Printf("%u\n",HAL_GetTick());
        }
        //如果等待200ms还没有获取到信号量则报错
        else{
            sBSP_UART_Debug_Printf("[ERR]AHRS错误:获取icm_data_ready_bin超时\n");
            Error_Handler();
        }
        //高精确度延时10ms
        // xTaskDelayUntil(&xLastWakeTime,10 / portTICK_PERIOD_MS);
    }
}

// if(input.gyro_x < 0.1 && input.gyro_z > -0.1){
//     input.gyro_x = 0;
// }
// if(input.gyro_y < 0.1 && input.gyro_y > -0.1){
//     input.gyro_y = 0;
// }
// if(input.gyro_z < 0.1 && input.gyro_z > -0.1){
//     input.gyro_z = 0;
// }



void sAPP_AHRS_ICMDataReadyCbISR(){
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    //通知数据就绪
	xSemaphoreGiveFromISR(icm_data_ready_bin, &xHigherPriorityTaskWoken);
    if(xHigherPriorityTaskWoken)portYIELD();
}




