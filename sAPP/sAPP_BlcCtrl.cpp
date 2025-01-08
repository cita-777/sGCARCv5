#include "sAPP_BlcCtrl.hpp"


/*这些是上电默认值*/
//机械中值
#define MECHINE_CENTER_ANGLE    (  1.0f)
//直立环初始值
#define STAND_KP                (-14.00f * 0.6f)
#define STAND_KD                (- 0.85f * 0.6f)
//速度环初始值
#define SPD_KP                  ( 0.30f)
#define SPD_KI                  (SPD_KP / 200.0f)
//转向环初始值
#define TURN_KP                 (  0.20f)
#define TURN_KD                 (  0.05f)
//0位置环初始值
#define ZERO_KP                 (120.00f)
#define ZERO_KI                 ( 30.00f)
#define ZERO_KD                 (  0.00f)
//轮胎半径
#define WHEEL_RADIUS            (0.0672f)



QueueHandle_t g_blc_ctrl_ahrs_queue;
static AHRS::AHRS_Data dat;


sAPP_BlcCtrl_Blc_t g_blc;
sAPP_BlcCtrl_Ctrl_t g_ctrl;
//保存导航信息
sAPP_CTRL_Nav_t g_nav;

sLib_IncPID_t pid_inc_pos;

//PD直立环
static inline float StandPDCtrler(float pitch_deg,float pitch_deg_spd);
//PI速度环
static float SpdPICtrler(float left_rpm,float right_rpm);
//PD转向环
static float TurnPDCtrler(float gyro_z);


//平衡控制初始化
void sAPP_BlcCtrl_Init(){
    //创建平衡控制算法需要的AHRS数据队列
    g_blc_ctrl_ahrs_queue = xQueueCreate(1,sizeof(AHRS::AHRS_Data));

    //设置各个参数的初始值
    g_blc.m_angle = MECHINE_CENTER_ANGLE;
    g_blc.stand_kp = STAND_KP;
    g_blc.stand_kd = STAND_KD;
    g_blc.spd_kp   = SPD_KP;
    g_blc.spd_ki   = SPD_KI;
    g_blc.turn_kd  = TURN_KD;
    g_blc.turn_kp  = TURN_KP;

    g_blc.inc_pos_kp = ZERO_KP;
    g_blc.inc_pos_ki = ZERO_KI;
    g_blc.inc_pos_kd = ZERO_KD;

    //上电禁用平衡
    g_ctrl.blc_en = 0;

    sLib_IncPIDInit(&pid_inc_pos,-200,200,1000,1000);
    sLib_IncPIDSetK(&pid_inc_pos,g_blc.inc_pos_kp, g_blc.inc_pos_ki, g_blc.inc_pos_kd);
    sLib_IncPIDSetTarget(&pid_inc_pos,0);
}

#include "sDRV_PS2.h"
#include "sBSP_UART.h"

void sAPP_BlcCtrl_Handler(){
    #define DT_S (0.005f)

    sDRV_PS2_t ps2;
    sDRV_PS2_GetData(&ps2);

    static float input_spd,input_head;
    input_spd = -(float)(ps2.leftY - 0x7F);
    input_head = -(float)(ps2.rightX - 0x7F);

    input_head = sLib_Fmap(input_head,-128,127,-180,180);

    if(input_head < 5 && input_head > -5){
        input_head = 0;
    }




    g_ctrl.tar_spd2 = sLib_Fmap(input_spd,-128,128,-60,60);

    if(g_ctrl.tar_spd2 < 1 && g_ctrl.tar_spd2 > -1){
        g_ctrl.tar_spd2 = 0;
    }


    g_ctrl.tar_head += input_head * DT_S;
    if(g_ctrl.tar_head > 180){
        g_ctrl.tar_head -= 360;
    }
    if(g_ctrl.tar_head < -180){
        g_ctrl.tar_head += 360;
    }
    

    //sBSP_UART_Debug_Printf("tar_spd: %6.1f,tar_head:%6.1f \n",g_ctrl.tar_spd,g_ctrl.tar_head);

    //如果没有启用平衡就不计算
    if(g_ctrl.blc_en == 0){
        //电机设置开路
        motor.setLBrake();
        motor.setRBrake();
        //sBSP_UART_Debug_Printf("NO\n");
        // goto PRINT;
        // sBSP_UART_Debug_Printf("%.2f,%.2f,%.2f,%.2f,%.4f,%.2f\n",g_blc.left_pwm,g_blc.right_pwm,dat.pitch,dat.gyr_x,x_velo,x_pos);

        return;
    }

    

    

    if(dat.pitch > 45 || dat.pitch < -45){
        sDRV_DRV8870_SetRightBrake(1);
        sDRV_DRV8870_SetLeftBrake(1);
        //关闭平衡开关
        g_ctrl.blc_en = 0;
        //Error_Handler();
    }

    g_blc.left_rpm  = motor.getLRPM();
    g_blc.right_rpm = motor.getRRPM();

    

    float inc_pos_out;

    //应用转向环
    g_blc.turn_out  = TurnPDCtrler (dat.yaw);

    if(fabs(g_blc.turn_out) < 10){
        //对编码器读到的转速值转换成线速度并积分
        g_nav.y_pos += (g_blc.left_rpm + g_blc.right_rpm) * ((2.0f * 3.1416 * WHEEL_RADIUS) / 60.0f) * DT_S;
    }

    if(g_ctrl.tar_spd2 < 1 && g_ctrl.tar_spd2 > -1){
        inc_pos_out = sLib_IncPIDUpdate(&pid_inc_pos,g_nav.y_pos,DT_S);
        g_ctrl.tar_spd = inc_pos_out;
    }else{
        // g_ctrl.tar_spd = g_ctrl.tar_spd2;
        g_ctrl.tar_spd = 0;
        g_nav.y_pos = 0;
        pid_inc_pos.integral = 0;
        inc_pos_out = 0;
    }

    //计算直立环
    g_blc.stand_out = StandPDCtrler(dat.pitch,dat.gyr_x);
    //计算速度环
    g_blc.spd_out   = SpdPICtrler  (g_blc.left_rpm,g_blc.right_rpm);

    
    

    //合并三环输出
    g_blc.left_pwm  = g_blc.stand_out + g_blc.spd_out + g_blc.turn_out - g_ctrl.tar_spd2;
    g_blc.right_pwm = g_blc.stand_out + g_blc.spd_out - g_blc.turn_out - g_ctrl.tar_spd2;




    //输出限幅
    sLib_FLimit(&g_blc.left_pwm,-100,100);
    sLib_FLimit(&g_blc.right_pwm,-100,100);
    
    motor.setLM(-g_blc.left_pwm);
    motor.setRM(-g_blc.right_pwm);

    // motor.setLM(g_blc.left_pwm);
    // motor.setRM(g_blc.right_pwm);


    PRINT:
    (void)0;

    //调试:平衡
    // sBSP_UART_Debug_Printf("%.2f,%.2f,%.2f,%.2f,%.4f,%.2f\n",dat.pitch,dat.gyr_x,g_blc.left_pwm,g_blc.right_pwm,motor.getLRPM(),motor.getRRPM());
    //dbg.printf("%.2f,%.2f,%.2f,%.2f,%.4f,%.2f\n",ahrs.pitch,g_blc.left_pwm,g_blc.right_pwm,motor.getLRPM(),motor.getRRPM());

    sBSP_UART_Debug_Printf("%.2f\n",inc_pos_out);

    // sBSP_UART_Debug_Printf("%.2f,%.2f,%.2f,%.2f,%.4f,%.2f\n",g_blc.left_pwm,g_blc.right_pwm,dat.pitch,dat.gyr_x,x_velo,x_pos);

}




//直立环PD控制,传参:俯仰角,俯仰角速度
static inline float StandPDCtrler(float pitch_deg,float pitch_deg_spd){
    //应用机械中值后,使用直立环PD控制器
    return g_blc.stand_kp * (pitch_deg - g_blc.m_angle) + g_blc.stand_kd * pitch_deg_spd;
}

//速度环PI控制,传参:左右电机编码器转速
static float SpdPICtrler(float left_rpm,float right_rpm){
    static float velocity, Encoder_Least, Encoder_bias;
    static float Encoder_Integral;

    //计算最新速度偏差
    //if(g_ctrl.tar_spd2 > 0){
        Encoder_Least = g_ctrl.tar_spd - (left_rpm + right_rpm);
    // }else{
    //     Encoder_Least = g_ctrl.tar_spd2 - (left_rpm + right_rpm);
    //     Encoder_Least = - Encoder_Least;
    // }
    
    //一阶低通滤波器,减缓速度变化
    Encoder_bias *= 0.86;
    Encoder_bias += Encoder_Least * 0.14;
    //积分出位移
    Encoder_Integral += Encoder_bias;
    //接收遥控器数据,控制前进后退
    Encoder_Integral += 0;
    //积分限幅
    sLib_FLimit(&Encoder_Integral, -1000, 1000);
    //计算速度PI
    velocity = -Encoder_bias * g_blc.spd_kp - Encoder_Integral * g_blc.spd_ki;
    return velocity;
}


float angleDifference(float angle1, float angle2) {
    float diff = angle2 - angle1;
    while (diff < -180.0) diff += 360.0;
    while (diff > 180.0) diff -= 360.0;
    return diff;
}

//转向环PD控制,传参:陀螺仪角度
static float TurnPDCtrler(float gyro_z){
    //转向值,用于返回
    static float turn_val;

    float error = angleDifference(g_ctrl.tar_head, gyro_z);

    //计算转向PD控制器
    //turn_val = (gyro_z - ctrl.tar_head) * blc.turn_kp + (gyro_z - ctrl.tar_head) * blc.turn_kd;

    turn_val = error * g_blc.turn_kp + error * g_blc.turn_kd;

    return turn_val;
}



void sAPP_BlcCtrl_CtrlTask(void* param){
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    int i = 0;
    for(;;){
        if(xQueueReceive(g_blc_ctrl_ahrs_queue,&dat,200)){
            //更新电机GMR编码器数据
            if(i >= 4){
                i = 0;
                motor.update();
            }
            i++;
            //调用平衡控制算法
            sAPP_BlcCtrl_Handler();
        }else{
            sBSP_UART_Debug_Printf("[ERR]BlcCtrl错误:获取g_blc_ctrl_ahrs_queue超时\n");
            Error_Handler();
        }

        // //高精确度延时10ms
        // xTaskDelayUntil(&xLastWakeTime,10 / portTICK_PERIOD_MS);
    }
}


//? LQR
    // //轮子转速 m/s
    // float left_velo = g_blc.left_rpm * ((2.0f * M_PI * WHEEL_RADIUS) / 60.0f);
    // float right_velo = g_blc.right_rpm * ((2.0f * M_PI * WHEEL_RADIUS) / 60.0f);
    // //x
    // static float x_velo;
    // x_velo = (left_velo + right_velo) / 2.0f;
    // //x位置 m
    // static float x_pos;
    // x_pos += x_velo * DT_S;

    // //angle rad/s
    // float omega_rads = dat.gyr_x * DEG2RAD;
    // //angle rad
    // float angle_rad  = (dat.pitch - MECHINE_CENTER_ANGLE) * DEG2RAD;
    // //turn velo rad/s
    // float turn_velo  = (g_blc.right_rpm - g_blc.left_rpm) / 161.0f / 1000;
    // //turn rad
    // static float turn_rad;
    // turn_rad += dat.gyr_z * DEG2RAD * DT_S;

    // //LQR-K
    // // static float K1=-77.4597, K2=-113.9570, K3=-357.2249, K4=-33.3211, K5=22.3607, K6=22.8301;
    // // static float K1=-22.3607, K2=-39.8701, K3=-393.5334, K4=-31.0357, K5=22.3607, K6=4.9891;
    // static float K1=-0.3607, K2=-0.6957, K3=-380.6007, K4=-40.3537, K5=22.3607, K6=5.0942;

    // //calculate LQR controller
    // float l_acc = K1 * x_pos + K2 * x_velo + K3 * angle_rad + K4 * omega_rads + K5 * turn_rad + K6 * (dat.gyr_z * DEG2RAD);
    // float r_acc = K1 * x_pos + K2 * x_velo + K3 * angle_rad + K4 * omega_rads - K5 * turn_rad - K6 * (dat.gyr_z * DEG2RAD);
    // l_acc = -l_acc;
    // r_acc = -r_acc;

    // float outL = x_velo + l_acc * DT_S;
    // float outR = x_velo + r_acc * DT_S;

    // g_blc.left_pwm  = outL * 116.0f;
    // g_blc.right_pwm = outR * 116.0f;


