/**
 * @file test_zdt_motor_protocol.h
 * @brief EMM V5.0步进电机驱动器协议测试程序头文件
 * @author Sightseer
 * @date 2025-01-26
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief EMM V5.0步进电机驱动器协议测试主函数
 * 
 * 该函数将测试修正后的协议实现，包括：
 * - 电机使能/禁用控制
 * - 速度模式控制
 * - 各种读取命令
 * - 解除堵转保护
 * 
 * 测试过程中会输出详细的调试信息，便于验证协议实现的正确性
 */
void test_zdt_motor_protocol();

#ifdef __cplusplus
}
#endif
