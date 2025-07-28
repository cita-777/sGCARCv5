#pragma once


#include "stm32f4xx_hal.h"


#include "sAPP_AHRS.hpp"
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

    // PD航向控制相关方法
    void  enableHeadingControl(bool enable);
    bool  isHeadingControlEnabled();
    void  setHeadingPDParams(float kp, float kd);
    void  setTargetHeading(float target_heading_deg);
    float getTargetHeading();
    float getCurrentHeading();

    // 距离控制相关方法
    void  enableDistanceControl(bool enable);
    bool  isDistanceControlEnabled();
    void  setTargetDistance(float target_distance_m);
    float getTargetDistance();
    float getCurrentDistance();
    void  resetDistance();

    // 高级运动控制方法
    void moveForwardWithHeading(float rpm, float target_heading_deg);
    void moveDistance(float rpm, float distance_m);
    void moveDistanceWithHeading(float rpm, float distance_m, float target_heading_deg);
    void turnToHeading(float target_heading_deg, float turn_rpm = 30.0f);

    // 控制状态查询
    bool  isMovementComplete();
    float getHeadingError();
    float getDistanceError();

private:
    // 闭环控制相关成员变量
    bool m_closed_loop_enabled;

    // 左右电机的PID控制器
    sLib_PosPID_t m_left_pid;
    sLib_PosPID_t m_right_pid;

    // 目标转速
    float m_left_target_rpm;
    float m_right_target_rpm;

    // PD航向控制相关成员变量
    bool  m_heading_control_enabled;
    float m_heading_kp;
    float m_heading_kd;
    float m_target_heading_deg;
    float m_prev_heading_error;

    // 距离控制相关成员变量
    bool  m_distance_control_enabled;
    float m_target_distance_m;
    float m_start_distance_left;
    float m_start_distance_right;

    // 运动状态控制
    enum MovementMode
    {
        MODE_MANUAL,             // 手动控制模式
        MODE_FORWARD_HEADING,    // 带航向的前进模式
        MODE_DISTANCE,           // 距离控制模式
        MODE_DISTANCE_HEADING,   // 距离+航向控制模式
        MODE_TURN_TO_HEADING     // 转向到目标角度模式
    };
    MovementMode m_movement_mode;
    bool         m_movement_complete;

    // 内部控制方法
    void updateLeftMotorControl(float dt_s);
    void updateRightMotorControl(float dt_s);

    // PD航向控制内部方法
    float calculateHeadingCorrection(float dt_s);
    float normalizeAngle(float angle_deg);
    float getAngleDifference(float target_deg, float current_deg);

    // 距离控制内部方法
    void  updateDistanceControl();
    float getAverageDistance();
};


extern sAPP_Motor motor;
