#include "sAPP_Motor.hpp"
#include "sBSP_UART.h"

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
}
