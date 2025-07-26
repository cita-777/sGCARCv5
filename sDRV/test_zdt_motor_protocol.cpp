/**
 * @file test_zdt_motor_protocol.cpp
 * @brief EMM V5.0步进电机驱动器协议测试程序
 * @author Sightseer
 * @date 2025-01-26
 * 
 * 根据官方协议文档测试修正后的命令实现
 */

#include "sDRV_zdt_motor.h"
#include "sDBG_Debug.h"

// 测试用的步进电机实例
static sDRV_ZDTMotor* test_motor = nullptr;

/**
 * @brief 等待命令完成并打印结果
 */
bool wait_for_command_complete(const char* cmd_name, uint32_t timeout_ms = 1000)
{
    uint32_t start_time = HAL_GetTick();
    
    while (!test_motor->is_recv_complete()) {
        if (HAL_GetTick() - start_time > timeout_ms) {
            dbg_printf("[ERROR] %s 命令超时\n", cmd_name);
            return false;
        }
        HAL_Delay(10);
    }
    
    int result = test_motor->get_recv_result();
    dbg_printf("[INFO] %s 命令完成，结果: %d (0x%08X)\n", cmd_name, result, result);
    
    return true;
}

/**
 * @brief 测试电机使能控制
 */
void test_motor_enable()
{
    dbg_printf("\n=== 测试电机使能控制 ===\n");
    
    // 测试使能电机
    dbg_printf("发送电机使能命令...\n");
    test_motor->ctrl_en(1);  // 使能
    wait_for_command_complete("电机使能");
    
    HAL_Delay(500);
    
    // 测试禁用电机
    dbg_printf("发送电机禁用命令...\n");
    test_motor->ctrl_en(0);  // 禁用
    wait_for_command_complete("电机禁用");
}

/**
 * @brief 测试速度模式控制
 */
void test_speed_mode()
{
    dbg_printf("\n=== 测试速度模式控制 ===\n");
    
    // 先使能电机
    test_motor->ctrl_en(1);
    wait_for_command_complete("电机使能");
    
    // 测试速度模式：CCW方向，100RPM，加速度档位5
    dbg_printf("发送速度模式命令 (CCW, 100RPM, 加速度5)...\n");
    test_motor->ctrl_speed_mode(1, 100, 5);  // CCW, 100RPM, 加速度5
    wait_for_command_complete("速度模式");
    
    HAL_Delay(2000);  // 运行2秒
    
    // 停止电机
    dbg_printf("发送电机禁用命令...\n");
    test_motor->ctrl_en(0);
    wait_for_command_complete("电机禁用");
}

/**
 * @brief 测试读取命令
 */
void test_read_commands()
{
    dbg_printf("\n=== 测试读取命令 ===\n");
    
    // 测试读取编码器值
    dbg_printf("读取编码器值...\n");
    test_motor->get_encoder_value();
    wait_for_command_complete("读取编码器值");
    
    // 测试读取电机状态
    dbg_printf("读取电机状态...\n");
    test_motor->get_en_status();
    wait_for_command_complete("读取电机状态");
    
    // 测试读取电机位置
    dbg_printf("读取电机位置...\n");
    test_motor->get_motor_position();
    wait_for_command_complete("读取电机位置");
    
    // 测试读取位置误差
    dbg_printf("读取位置误差...\n");
    test_motor->get_motor_position_err();
    wait_for_command_complete("读取位置误差");
}

/**
 * @brief 测试解除堵转保护
 */
void test_cancel_protection()
{
    dbg_printf("\n=== 测试解除堵转保护 ===\n");
    
    dbg_printf("发送解除堵转保护命令...\n");
    test_motor->cancel_motor_protect();
    wait_for_command_complete("解除堵转保护");
}

/**
 * @brief 主测试函数
 */
void test_zdt_motor_protocol()
{
    dbg_printf("\n========================================\n");
    dbg_printf("EMM V5.0步进电机驱动器协议测试开始\n");
    dbg_printf("========================================\n");
    
    // 创建步进电机实例 (地址1, 115200波特率, 校验码0x6B)
    test_motor = new sDRV_ZDTMotor(1, 115200, 0x6B);
    
    if (!test_motor) {
        dbg_printf("[ERROR] 无法创建步进电机实例\n");
        return;
    }
    
    dbg_printf("[INFO] 步进电机实例创建成功\n");
    
    // 等待初始化完成
    HAL_Delay(1000);
    
    // 依次执行各项测试
    test_cancel_protection();    // 先解除可能的保护状态
    test_read_commands();        // 测试读取命令
    test_motor_enable();         // 测试使能控制
    test_speed_mode();           // 测试速度模式
    
    dbg_printf("\n========================================\n");
    dbg_printf("EMM V5.0步进电机驱动器协议测试完成\n");
    dbg_printf("========================================\n");
    
    // 清理资源
    delete test_motor;
    test_motor = nullptr;
}
