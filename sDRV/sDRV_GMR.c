#include "sDRV_GMR.h"

#include "sBSP_TIM.h"

// 已删除bool定义，改用unsigned char

// TIM3 CH1 CH2 -> Left电机
// TIM4 CH1 CH2 -> Right电机

// 500线GMR转一圈30K个脉冲
// 好像不能使用双边沿模式,也就是四倍频模式

// 241011 PM05:24
// 新增功能，在Handler里，自适应时间间隔，使用HAL_GetTick，原来是define的固定间隔，如果调用时间不一致就会数据不对
// 但是还是得注意数据溢出问题


#define GMR_ROUND_PLUSE (30000)
// #define GMR_ROUND_PLUSE     (1464)


float right_rpm;
float left_rpm;

// 距离累计变量（单位：米）
float left_distance  = 0.0f;
float right_distance = 0.0f;

void sDRV_GMR_Init()
{
    sBSP_TIM_GMRL_Init();
    sBSP_TIM_GMRL_SetEN(1);
    sBSP_TIM_GMRL_Set(32767);

    sBSP_TIM_GMRR_Init();
    sBSP_TIM_GMRR_SetEN(1);
    sBSP_TIM_GMRR_Set(32767);
}


float sDRV_GMR_GetLeftRPM()
{
    //! 两个电机方向是相反的
    return -left_rpm;
}

float sDRV_GMR_GetRightRPM()
{
    return right_rpm;
}

// 修复的RPM计算算法，提高响应速度，简化计算
void sDRV_GMR_Handler()
{
    static unsigned long prev_time   = 0;
    static unsigned long prev_left   = 32767;
    static unsigned long prev_right  = 32767;
    static unsigned char initialized = 0;

    unsigned long now_time      = HAL_GetTick();
    unsigned long current_left  = sBSP_TIM_GMRL_Get();
    unsigned long current_right = sBSP_TIM_GMRR_Get();

    if (!initialized)
    {
        prev_left   = current_left;
        prev_right  = current_right;
        prev_time   = now_time;
        initialized = 1;
        left_rpm    = 0.0f;
        right_rpm   = 0.0f;
        return;
    }

    float dt = (float)(now_time - prev_time) / 1000.0f;   // 转换为秒
    if (dt < 0.010f) return;                              // 回到10ms间隔，保证稳定性

    // 计算差值 - 简化版本
    long left_diff  = (long)current_left - (long)prev_left;
    long right_diff = (long)current_right - (long)prev_right;

    // 简单有效的16位溢出处理
    // 如果差值的绝对值大于32767，说明发生了溢出
    if (left_diff > 32767)
    {
        left_diff -= 65536;
    }
    else if (left_diff < -32767)
    {
        left_diff += 65536;
    }

    if (right_diff > 32767)
    {
        right_diff -= 65536;
    }
    else if (right_diff < -32767)
    {
        right_diff += 65536;
    }

    // 计算RPM - 确保电机正功率对应正RPM
    // 根据您的要求：motor调用函数的速度是正，编码器就是正
    left_rpm  = (float)left_diff / dt / GMR_ROUND_PLUSE * 60.0f;    // 左电机去掉负号
    right_rpm = (float)right_diff / dt / GMR_ROUND_PLUSE * 60.0f;   // 右电机保持不变

    // 计算距离增量（假设轮子直径为6.5cm）
    float wheel_circumference  = 3.14159f * 0.065f;                                           // 轮子周长，单位：米
    float left_distance_delta  = (float)left_diff / GMR_ROUND_PLUSE * wheel_circumference;    // 左电机去掉负号
    float right_distance_delta = (float)right_diff / GMR_ROUND_PLUSE * wheel_circumference;   // 右电机保持不变

    // 累计距离
    left_distance += left_distance_delta;
    right_distance += right_distance_delta;

    // 更新历史值
    prev_left  = current_left;
    prev_right = current_right;
    prev_time  = now_time;
}

// 距离查询API
float sDRV_GMR_GetLeftDistance()
{
    return left_distance;
}

float sDRV_GMR_GetRightDistance()
{
    return right_distance;
}

// 重置距离计数
void sDRV_GMR_ResetDistance()
{
    left_distance  = 0.0f;
    right_distance = 0.0f;
}
