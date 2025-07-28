#include "sAPP_Motor.hpp"
#include "FreeRTOS.h"
#include "sBSP_UART.h"
#include "semphr.h"

// 声明数学函数
extern "C" {
float fabsf(float x);
}

sAPP_Motor motor;

// 默认PID参数（参考sAPP_BlcCtrl.cpp中的速度环参数）
#define DEFAULT_SPEED_KP (0.8f)    // 比原来的速度环参数稍大，因为这是直接控制单个电机
#define DEFAULT_SPEED_KI (0.1f)    // 积分项，用于消除稳态误差
#define DEFAULT_SPEED_KD (0.05f)   // 微分项，提高响应速度

sAPP_Motor::sAPP_Motor()
{
    m_closed_loop_enabled = false;
    m_left_target_rpm     = 0.0f;
    m_right_target_rpm    = 0.0f;

    // 初始化PD航向控制参数
    m_heading_control_enabled = false;
    m_heading_kp              = 0.5f;
    m_heading_kd              = 0.1f;
    m_target_heading_deg      = 0.0f;
    m_prev_heading_error      = 0.0f;

    // 初始化距离控制参数
    m_distance_control_enabled = false;
    m_target_distance_m        = 0.0f;
    m_start_distance_left      = 0.0f;
    m_start_distance_right     = 0.0f;

    // 初始化运动状态
    m_movement_mode     = MODE_MANUAL;
    m_movement_complete = false;
}

sAPP_Motor::~sAPP_Motor() {}

int sAPP_Motor::init()
{
    sDRV_DRV8870_Init();
    sDRV_GMR_Init();

    // 初始化PID控制器
    // 输出范围：-100到100（PWM百分比）
    // 积分限幅：-50到50（防止积分饱和）
    sLib_PosPIDInit(&m_left_pid, -100.0f, 100.0f, -50.0f, 50.0f);
    sLib_PosPIDInit(&m_right_pid, -100.0f, 100.0f, -50.0f, 50.0f);

    // 设置默认PID参数
    sLib_PosPIDSetK(&m_left_pid, DEFAULT_SPEED_KP, DEFAULT_SPEED_KI, DEFAULT_SPEED_KD);
    sLib_PosPIDSetK(&m_right_pid, DEFAULT_SPEED_KP, DEFAULT_SPEED_KI, DEFAULT_SPEED_KD);

    // 设置初始目标转速为0
    sLib_PosPIDSetTarget(&m_left_pid, 0.0f);
    sLib_PosPIDSetTarget(&m_right_pid, 0.0f);

    return 0;
}


void sAPP_Motor::setLM(float percent)
{
    sDRV_DRV8870_SetLeftPct(-percent);
}

float sAPP_Motor::getLRPM()
{
    return sDRV_GMR_GetLeftRPM();
}

void sAPP_Motor::setLBrake(bool is_short)
{
    sDRV_DRV8870_SetLeftBrake(is_short);
}


void sAPP_Motor::setRM(float percent)
{
    sDRV_DRV8870_SetRightPct(-percent);
}

float sAPP_Motor::getRRPM()
{
    return sDRV_GMR_GetRightRPM();
}

void sAPP_Motor::setRBrake(bool is_short)
{
    sDRV_DRV8870_SetRightBrake(is_short);
}



void sAPP_Motor::update()
{
    sDRV_GMR_Handler();
}

// 新增的闭环速度控制方法实现
void sAPP_Motor::setLeftTargetRPM(float target_rpm)
{
    m_left_target_rpm = target_rpm;
    sLib_PosPIDSetTarget(&m_left_pid, target_rpm);
}

void sAPP_Motor::setRightTargetRPM(float target_rpm)
{
    m_right_target_rpm = target_rpm;
    sLib_PosPIDSetTarget(&m_right_pid, target_rpm);
}

void sAPP_Motor::setTargetRPM(float left_rpm, float right_rpm)
{
    setLeftTargetRPM(left_rpm);
    setRightTargetRPM(right_rpm);
}

float sAPP_Motor::getLeftTargetRPM()
{
    return m_left_target_rpm;
}

float sAPP_Motor::getRightTargetRPM()
{
    return m_right_target_rpm;
}

void sAPP_Motor::enableClosedLoopControl(bool enable)
{
    m_closed_loop_enabled = enable;

    if (!enable)
    {
        // 禁用闭环控制时，停止电机
        setLM(0.0f);
        setRM(0.0f);
    }
}

bool sAPP_Motor::isClosedLoopEnabled()
{
    return m_closed_loop_enabled;
}

void sAPP_Motor::setLeftPIDParams(float kp, float ki, float kd)
{
    sLib_PosPIDSetK(&m_left_pid, kp, ki, kd);
}

void sAPP_Motor::setRightPIDParams(float kp, float ki, float kd)
{
    sLib_PosPIDSetK(&m_right_pid, kp, ki, kd);
}

void sAPP_Motor::setPIDParams(float kp, float ki, float kd)
{
    setLeftPIDParams(kp, ki, kd);
    setRightPIDParams(kp, ki, kd);
}

// 闭环控制更新函数
void sAPP_Motor::updateClosedLoopControl(float dt_s)
{
    if (!m_closed_loop_enabled)
    {
        return;
    }

    // 更新距离控制状态
    updateDistanceControl();

    // 计算航向校正
    float heading_correction = calculateHeadingCorrection(dt_s);

    // 根据运动模式应用航向校正
    float left_rpm_adjustment  = 0.0f;
    float right_rpm_adjustment = 0.0f;

    if (m_heading_control_enabled)
    {
        // 航向校正：正值表示需要左转，负值表示需要右转
        left_rpm_adjustment  = -heading_correction;   // 左轮减速
        right_rpm_adjustment = heading_correction;    // 右轮加速

        // 对于转向模式，需要特殊处理
        if (m_movement_mode == MODE_TURN_TO_HEADING)
        {
            float heading_error = getHeadingError();
            if (fabsf(heading_error) < 2.0f)
            {   // 2度精度
                stop();
                m_movement_complete = true;
                return;
            }

            // 动态调整转向方向
            if (heading_error > 0)
            {
                // 需要左转
                sLib_PosPIDSetTarget(&m_left_pid, -30.0f);
                sLib_PosPIDSetTarget(&m_right_pid, 30.0f);
            }
            else
            {
                // 需要右转
                sLib_PosPIDSetTarget(&m_left_pid, 30.0f);
                sLib_PosPIDSetTarget(&m_right_pid, -30.0f);
            }
        }
        else
        {
            // 对于其他模式，在基础转速上应用航向校正
            sLib_PosPIDSetTarget(&m_left_pid, m_left_target_rpm + left_rpm_adjustment);
            sLib_PosPIDSetTarget(&m_right_pid, m_right_target_rpm + right_rpm_adjustment);
        }
    }

    // 更新左右电机的闭环控制
    updateLeftMotorControl(dt_s);
    updateRightMotorControl(dt_s);
}

// 左电机控制更新
void sAPP_Motor::updateLeftMotorControl(float dt_s)
{
    // 获取当前转速反馈
    float current_rpm = getLRPM();

    // 计算PID输出
    float pid_output = sLib_PosPIDUpdate(&m_left_pid, current_rpm, dt_s);

    // 应用PID输出到电机（注意：这里直接使用原有的PWM控制方法）
    setLM(pid_output);
}

// 右电机控制更新
void sAPP_Motor::updateRightMotorControl(float dt_s)
{
    // 获取当前转速反馈
    float current_rpm = getRRPM();

    // 计算PID输出
    float pid_output = sLib_PosPIDUpdate(&m_right_pid, current_rpm, dt_s);

    // 应用PID输出到电机（注意：这里直接使用原有的PWM控制方法）
    setRM(pid_output);
}

// 便捷的运动控制方法实现
void sAPP_Motor::moveForward(float rpm)
{
    if (m_closed_loop_enabled)
    {
        setTargetRPM(rpm, rpm);
    }
    else
    {
        // 如果闭环控制未启用，则使用开环控制
        float pwm_percent = rpm * 1.5f;   // 简单的转换，可能需要根据实际情况调整
        sLib_FLimit(&pwm_percent, -100.0f, 100.0f);
        setLM(pwm_percent);
        setRM(pwm_percent);
    }
}

void sAPP_Motor::moveBackward(float rpm)
{
    moveForward(-rpm);
}

void sAPP_Motor::turnLeft(float rpm)
{
    if (m_closed_loop_enabled)
    {
        setTargetRPM(-rpm, rpm);   // 左轮反转，右轮正转
    }
    else
    {
        float pwm_percent = rpm * 1.5f;
        sLib_FLimit(&pwm_percent, -100.0f, 100.0f);
        setLM(-pwm_percent);
        setRM(pwm_percent);
    }
}

void sAPP_Motor::turnRight(float rpm)
{
    if (m_closed_loop_enabled)
    {
        setTargetRPM(rpm, -rpm);   // 左轮正转，右轮反转
    }
    else
    {
        float pwm_percent = rpm * 1.5f;
        sLib_FLimit(&pwm_percent, -100.0f, 100.0f);
        setLM(pwm_percent);
        setRM(-pwm_percent);
    }
}

void sAPP_Motor::stop()
{
    if (m_closed_loop_enabled)
    {
        setTargetRPM(0.0f, 0.0f);
    }
    else
    {
        setLM(0.0f);
        setRM(0.0f);
    }
    m_movement_mode     = MODE_MANUAL;
    m_movement_complete = true;
}

// ==================== PD航向控制相关方法 ====================

void sAPP_Motor::enableHeadingControl(bool enable)
{
    m_heading_control_enabled = enable;
    if (enable)
    {
        // 重置PD控制器状态
        m_prev_heading_error = 0.0f;
        // 设置当前航向为目标航向
        m_target_heading_deg = getCurrentHeading();
    }
}

bool sAPP_Motor::isHeadingControlEnabled()
{
    return m_heading_control_enabled;
}

void sAPP_Motor::setHeadingPDParams(float kp, float kd)
{
    m_heading_kp = kp;
    m_heading_kd = kd;
}

void sAPP_Motor::setTargetHeading(float target_heading_deg)
{
    m_target_heading_deg = normalizeAngle(target_heading_deg);
    m_prev_heading_error = 0.0f;   // 重置微分项
}

float sAPP_Motor::getTargetHeading()
{
    return m_target_heading_deg;
}

float sAPP_Motor::getCurrentHeading()
{
    // 获取AHRS数据
    if (xSemaphoreTake(ahrs.output.lock, 10) == pdTRUE)
    {
        float current_yaw = ahrs.output.yaw;
        xSemaphoreGive(ahrs.output.lock);
        return current_yaw;
    }
    return 0.0f;   // 如果无法获取数据，返回0
}

// ==================== 距离控制相关方法 ====================

void sAPP_Motor::enableDistanceControl(bool enable)
{
    m_distance_control_enabled = enable;
    if (enable)
    {
        // 记录起始距离
        m_start_distance_left  = sDRV_GMR_GetLeftDistance();
        m_start_distance_right = sDRV_GMR_GetRightDistance();
    }
}

bool sAPP_Motor::isDistanceControlEnabled()
{
    return m_distance_control_enabled;
}

void sAPP_Motor::setTargetDistance(float target_distance_m)
{
    m_target_distance_m = target_distance_m;
    // 重新记录起始距离
    m_start_distance_left  = sDRV_GMR_GetLeftDistance();
    m_start_distance_right = sDRV_GMR_GetRightDistance();
}

float sAPP_Motor::getTargetDistance()
{
    return m_target_distance_m;
}

float sAPP_Motor::getCurrentDistance()
{
    return getAverageDistance() - (m_start_distance_left + m_start_distance_right) / 2.0f;
}

void sAPP_Motor::resetDistance()
{
    sDRV_GMR_ResetDistance();
    m_start_distance_left  = 0.0f;
    m_start_distance_right = 0.0f;
}

// ==================== 高级运动控制方法 ====================

void sAPP_Motor::moveForwardWithHeading(float rpm, float target_heading_deg)
{
    m_movement_mode     = MODE_FORWARD_HEADING;
    m_movement_complete = false;

    setTargetHeading(target_heading_deg);
    enableHeadingControl(true);

    // 设置基础前进速度
    setTargetRPM(rpm, rpm);
}

void sAPP_Motor::moveDistance(float rpm, float distance_m)
{
    m_movement_mode     = MODE_DISTANCE;
    m_movement_complete = false;

    setTargetDistance(distance_m);
    enableDistanceControl(true);

    // 设置前进速度
    if (distance_m > 0)
    {
        setTargetRPM(rpm, rpm);
    }
    else
    {
        setTargetRPM(-rpm, -rpm);
    }
}

void sAPP_Motor::moveDistanceWithHeading(float rpm, float distance_m, float target_heading_deg)
{
    m_movement_mode     = MODE_DISTANCE_HEADING;
    m_movement_complete = false;

    setTargetDistance(distance_m);
    setTargetHeading(target_heading_deg);
    enableDistanceControl(true);
    enableHeadingControl(true);

    // 设置前进速度
    if (distance_m > 0)
    {
        setTargetRPM(rpm, rpm);
    }
    else
    {
        setTargetRPM(-rpm, -rpm);
    }
}

void sAPP_Motor::turnToHeading(float target_heading_deg, float turn_rpm)
{
    m_movement_mode     = MODE_TURN_TO_HEADING;
    m_movement_complete = false;

    setTargetHeading(target_heading_deg);
    enableHeadingControl(true);

    // 初始转向方向判断
    float current_heading = getCurrentHeading();
    float angle_diff      = getAngleDifference(target_heading_deg, current_heading);

    if (angle_diff > 0)
    {
        // 需要左转
        setTargetRPM(-turn_rpm, turn_rpm);
    }
    else
    {
        // 需要右转
        setTargetRPM(turn_rpm, -turn_rpm);
    }
}

// ==================== 控制状态查询方法 ====================

bool sAPP_Motor::isMovementComplete()
{
    return m_movement_complete;
}

float sAPP_Motor::getHeadingError()
{
    float current_heading = getCurrentHeading();
    return getAngleDifference(m_target_heading_deg, current_heading);
}

float sAPP_Motor::getDistanceError()
{
    return m_target_distance_m - getCurrentDistance();
}

// ==================== 内部辅助方法实现 ====================

float sAPP_Motor::calculateHeadingCorrection(float dt_s)
{
    if (!m_heading_control_enabled)
    {
        return 0.0f;
    }

    float current_heading = getCurrentHeading();
    float error           = getAngleDifference(m_target_heading_deg, current_heading);

    // PD控制器计算
    float derivative = (error - m_prev_heading_error) / dt_s;
    float output     = m_heading_kp * error + m_heading_kd * derivative;

    // 更新上一次误差
    m_prev_heading_error = error;

    // 限制输出范围
    if (output > 50.0f) output = 50.0f;
    if (output < -50.0f) output = -50.0f;

    return output;
}

float sAPP_Motor::normalizeAngle(float angle_deg)
{
    while (angle_deg > 180.0f) angle_deg -= 360.0f;
    while (angle_deg < -180.0f) angle_deg += 360.0f;
    return angle_deg;
}

float sAPP_Motor::getAngleDifference(float target_deg, float current_deg)
{
    float diff = target_deg - current_deg;
    return normalizeAngle(diff);
}

void sAPP_Motor::updateDistanceControl()
{
    if (!m_distance_control_enabled)
    {
        return;
    }

    float current_distance = getCurrentDistance();
    float distance_error   = m_target_distance_m - current_distance;

    // 检查是否到达目标距离
    if (fabsf(distance_error) < 0.05f)
    {   // 5cm精度
        if (m_movement_mode == MODE_DISTANCE || m_movement_mode == MODE_DISTANCE_HEADING)
        {
            stop();
            m_movement_complete = true;
        }
    }
}

float sAPP_Motor::getAverageDistance()
{
    return (sDRV_GMR_GetLeftDistance() + sDRV_GMR_GetRightDistance()) / 2.0f;
}
