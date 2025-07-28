#pragma once


#include "stm32f4xx_hal.h"


#include "sDRV_DRV8870.h"
#include "sDRV_GMR.h"
#include "sLib_Ctrler.h"

class sAPP_Motor
{
public:
    sAPP_Motor();
    ~sAPP_Motor();

    int init();

    // 原有的直接PWM控制方法（保留用于兼容性）
    void  setLM(float percent);
    float getLRPM();
    void  setLBrake(bool is_short = false);

    void  setRM(float percent);
    float getRRPM();
    void  setRBrake(bool is_short = false);

    // 新增的闭环速度控制方法
    void setLeftTargetRPM(float target_rpm);
    void setRightTargetRPM(float target_rpm);
    void setTargetRPM(float left_rpm, float right_rpm);

    float getLeftTargetRPM();
    float getRightTargetRPM();

    // 启用/禁用闭环控制
    void enableClosedLoopControl(bool enable);
    bool isClosedLoopEnabled();

    // PID参数设置
    void setLeftPIDParams(float kp, float ki, float kd);
    void setRightPIDParams(float kp, float ki, float kd);
    void setPIDParams(float kp, float ki, float kd);

    void update();

    // 闭环控制更新（需要定期调用）
    void updateClosedLoopControl(float dt_s);

    // 便捷的运动控制方法
    void moveForward(float rpm);    // 前进，指定转速
    void moveBackward(float rpm);   // 后退，指定转速
    void turnLeft(float rpm);       // 左转，指定转速
    void turnRight(float rpm);      // 右转，指定转速
    void stop();                    // 停止

private:
    // 闭环控制相关成员变量
    bool m_closed_loop_enabled;

    // 左右电机的PID控制器
    sLib_PosPID_t m_left_pid;
    sLib_PosPID_t m_right_pid;

    // 目标转速
    float m_left_target_rpm;
    float m_right_target_rpm;

    // 内部控制方法
    void updateLeftMotorControl(float dt_s);
    void updateRightMotorControl(float dt_s);
};


extern sAPP_Motor motor;
