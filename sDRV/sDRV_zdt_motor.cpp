/*
 * Copyright 2023 Meieryang
 * Modified for STM32F405 by sGCARC Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Github URL: git@github.com:Meieryang/zdt-motor-lib.git
 *
 * Software version: 0.2 - STM32F405 Port
 */

#include "sDRV_zdt_motor.h"

// 使用BSP层的UART3 API (PB10/PB11)

// 静态实例指针定义
sDRV_ZDTMotor* sDRV_ZDTMotor::instance = 0;

sDRV_ZDTMotor::sDRV_ZDTMotor(unsigned char address, unsigned int baud_rate, unsigned char data_check_byte)
    : address(address)
    , baud_rate(baud_rate)
    , data_check_byte(data_check_byte)
    , recv_complete(true)   // 初始状态为完成，避免误判
    , recv_result(0)
    , expected_recv_length(0)
{
    // 设置静态实例指针
    instance = this;
    init();
}

int sDRV_ZDTMotor::init()
{
    // 使用BSP层的UART3 API (PB10/PB11)
    // UART3配置: PB10=TX, PB11=RX
    if (sBSP_UART_Motor_Init(baud_rate) != 0)
    {
        return -1;
    }

    // 设置UART接收回调
    sBSP_UART_Motor_RecvBegin(uart_recv_callback);
    dbg_printf("[INFO]ZDTMotor: UART3接收回调已设置\n");

    return 0;
}

// 重载不同参数数量的send_data函数
bool sDRV_ZDTMotor::send_data(unsigned char cmd)
{
    unsigned char hex_data[4];
    hex_data[0] = address;
    hex_data[1] = cmd;
    hex_data[2] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (3字节): ");
    for (int i = 0; i < 3; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 3);
    return true;
}

bool sDRV_ZDTMotor::send_data(unsigned char cmd, unsigned char p1)
{
    unsigned char hex_data[5];
    hex_data[0] = address;
    hex_data[1] = cmd;
    hex_data[2] = p1;
    hex_data[3] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (4字节): ");
    for (int i = 0; i < 4; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 4);
    return true;
}

bool sDRV_ZDTMotor::send_data(unsigned char cmd, unsigned char p1, unsigned char p2)
{
    unsigned char hex_data[6];
    hex_data[0] = address;
    hex_data[1] = cmd;
    hex_data[2] = p1;
    hex_data[3] = p2;
    hex_data[4] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (5字节): ");
    for (int i = 0; i < 5; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 5);
    return true;
}

bool sDRV_ZDTMotor::send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3)
{
    unsigned char hex_data[7];
    hex_data[0] = address;
    hex_data[1] = cmd;
    hex_data[2] = p1;
    hex_data[3] = p2;
    hex_data[4] = p3;
    hex_data[5] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (6字节): ");
    for (int i = 0; i < 6; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 6);
    return true;
}

bool sDRV_ZDTMotor::send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4)
{
    unsigned char hex_data[8];
    hex_data[0] = address;
    hex_data[1] = cmd;
    hex_data[2] = p1;
    hex_data[3] = p2;
    hex_data[4] = p3;
    hex_data[5] = p4;
    hex_data[6] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (7字节): ");
    for (int i = 0; i < 7; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 7);
    return true;
}

bool sDRV_ZDTMotor::send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4,
                              unsigned char p5)
{
    unsigned char hex_data[9];
    hex_data[0] = address;
    hex_data[1] = cmd;
    hex_data[2] = p1;
    hex_data[3] = p2;
    hex_data[4] = p3;
    hex_data[5] = p4;
    hex_data[6] = p5;
    hex_data[7] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (8字节): ");
    for (int i = 0; i < 8; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 8);
    return true;
}

bool sDRV_ZDTMotor::send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4,
                              unsigned char p5, unsigned char p6)
{
    unsigned char hex_data[10];
    hex_data[0] = address;
    hex_data[1] = cmd;
    hex_data[2] = p1;
    hex_data[3] = p2;
    hex_data[4] = p3;
    hex_data[5] = p4;
    hex_data[6] = p5;
    hex_data[7] = p6;
    hex_data[8] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (9字节): ");
    for (int i = 0; i < 9; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 9);
    return true;
}

// 添加更多参数的重载函数用于位置模式控制
bool sDRV_ZDTMotor::send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4,
                              unsigned char p5, unsigned char p6, unsigned char p7, unsigned char p8, unsigned char p9)
{
    unsigned char hex_data[12];
    hex_data[0]  = address;
    hex_data[1]  = cmd;
    hex_data[2]  = p1;
    hex_data[3]  = p2;
    hex_data[4]  = p3;
    hex_data[5]  = p4;
    hex_data[6]  = p5;
    hex_data[7]  = p6;
    hex_data[8]  = p7;
    hex_data[9]  = p8;
    hex_data[10] = p9;
    hex_data[11] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (12字节): ");
    for (int i = 0; i < 12; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 12);
    return true;
}

// 添加10个参数的重载函数用于位置模式控制
bool sDRV_ZDTMotor::send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4,
                              unsigned char p5, unsigned char p6, unsigned char p7, unsigned char p8, unsigned char p9,
                              unsigned char p10)
{
    unsigned char hex_data[13];
    hex_data[0]  = address;
    hex_data[1]  = cmd;
    hex_data[2]  = p1;
    hex_data[3]  = p2;
    hex_data[4]  = p3;
    hex_data[5]  = p4;
    hex_data[6]  = p5;
    hex_data[7]  = p6;
    hex_data[8]  = p7;
    hex_data[9]  = p8;
    hex_data[10] = p9;
    hex_data[11] = p10;
    hex_data[12] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (13字节): ");
    for (int i = 0; i < 13; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, 13);
    return true;
}

// 通用数组参数发送函数
bool sDRV_ZDTMotor::send_data_array(unsigned char cmd, unsigned char* params, unsigned char param_count)
{
    unsigned char hex_data[32];   // 最大支持32字节
    unsigned char index = 0;

    hex_data[index++] = address;
    hex_data[index++] = cmd;

    // 添加参数
    for (unsigned char i = 0; i < param_count; i++)
    {
        hex_data[index++] = params[i];
    }

    hex_data[index++] = data_check_byte;

    dbg_printf("[INFO]ZDTMotor: 发送数据 (%d字节): ", index);
    for (int i = 0; i < index; i++)
    {
        dbg_printf("0x%02X ", hex_data[i]);
    }
    dbg_printf("\n");

    sBSP_UART_Motor_SendBytes(hex_data, index);
    return true;
}

signed int sDRV_ZDTMotor::receive_data(signed char expected_length)
{
    dbg_printf("[INFO]ZDTMotor: 准备接收，期望长度=%d\n", expected_length);

    // 设置期望接收长度
    expected_recv_length = expected_length;
    recv_complete        = false;
    recv_result          = RT_WARNING;

    // 注意：不在这里启动接收，接收应该一直处于活跃状态
    dbg_printf("[INFO]ZDTMotor: 接收状态已准备，等待数据\n");

    // 返回一个特殊值表示正在等待接收
    return RT_WAITING;
}

// 静态回调函数实现
void sDRV_ZDTMotor::uart_recv_callback(char* data, unsigned short size)
{
    if (instance != 0)
    {
        instance->process_received_data(data, size);
    }

    // 关键：重新启动接收，就像uart_recied函数一样！
    sBSP_UART_Motor_RecvBegin(uart_recv_callback);
}

// 处理接收到的数据
void sDRV_ZDTMotor::process_received_data(char* data, unsigned short size)
{
    dbg_printf("[INFO]ZDTMotor: 收到数据 (%d字节): ", size);
    for (int i = 0; i < size; i++)
    {
        dbg_printf("0x%02X ", (unsigned char)data[i]);
    }
    dbg_printf("\n");

    // 如果当前没有等待接收（expected_recv_length为0），忽略这个数据
    if (expected_recv_length == 0)
    {
        dbg_printf("[INFO]ZDTMotor: 忽略意外数据（当前未等待接收）\n");
        return;
    }

    // 检查数据长度是否匹配期望
    if (size != expected_recv_length)
    {
        dbg_printf("[WARN]ZDTMotor: 数据长度不匹配，期望%d，收到%d\n", expected_recv_length, size);
        recv_complete        = true;
        recv_result          = RT_WARNING;
        expected_recv_length = 0;   // 重置期望长度
        return;
    }

    // 验证地址
    if ((unsigned char)data[0] != address)
    {
        dbg_printf("[WARN]ZDTMotor: 地址不匹配，期望0x%02X，收到0x%02X\n", address, (unsigned char)data[0]);
        recv_complete = true;
        recv_result   = RT_WARNING;
        return;
    }

    // 验证校验码
    if ((unsigned char)data[size - 1] != data_check_byte)
    {
        dbg_printf(
            "[WARN]ZDTMotor: 校验码不匹配，期望0x%02X，收到0x%02X\n", data_check_byte, (unsigned char)data[size - 1]);
        recv_complete = true;
        recv_result   = RT_WARNING;
        return;
    }

    // 根据协议解析响应数据
    unsigned char cmd_code = (unsigned char)data[1];   // 命令码

    if (size == 4)
    {
        // 4字节响应：地址 + 命令码 + 状态 + 校验码
        unsigned char status = (unsigned char)data[2];
        dbg_printf("[INFO]ZDTMotor: 命令0x%02X响应，状态=0x%02X", cmd_code, status);

        if (cmd_code == 0x3A)
        {
            // 电机状态标志位解析
            dbg_printf(" (状态标志位)\n");
            dbg_printf("  - 电机使能: %s\n", (status & 0x01) ? "是" : "否");
            dbg_printf("  - 电机到位: %s\n", (status & 0x02) ? "是" : "否");
            dbg_printf("  - 电机堵转: %s\n", (status & 0x04) ? "是" : "否");
            dbg_printf("  - 堵转保护: %s\n", (status & 0x08) ? "是" : "否");
            recv_result = status;   // 返回原始状态值
        }
        else if (status == 0x02)
        {
            dbg_printf(" (成功)\n");
            recv_result = RT_SUCCESS;
        }
        else if (status == 0xE2)
        {
            dbg_printf(" (条件不满足)\n");
            recv_result = RT_WARNING;
        }
        else if (status == 0xEE)
        {
            dbg_printf(" (错误命令)\n");
            recv_result = RT_ERROR;
        }
        else
        {
            dbg_printf(" (数据值)\n");
            recv_result = status;   // 返回数据值
        }
    }
    else
    {
        // 多字节数据响应：地址 + 命令码 + 数据 + 校验码
        signed int receivedData = 0;
        for (int i = 2; i < size - 1; i++)
        {   // 从第3个字节开始是数据
            receivedData = (receivedData << 8) | (unsigned char)data[i];
        }
        dbg_printf("[INFO]ZDTMotor: 命令0x%02X数据响应，数据=0x%08X\n", cmd_code, receivedData);
        recv_result = receivedData;
    }

    recv_complete        = true;
    expected_recv_length = 0;   // 重置期望长度
}

unsigned char sDRV_ZDTMotor::cancel_motor_protect()
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收
    // 根据协议：地址 + 0x0E + 0x52 + 校验字节
    send_data(0x0E, 0x52);
    return RT_WAITING;   // 返回等待状态
}

uint16_t sDRV_ZDTMotor::get_encoder_value()
{
    receive_data(_5_BYTES);   // 先准备接收：地址+命令+2字节编码器值+校验码
    send_data(CMD_GET_ENCODER_VALUE);
    return RT_WAITING;   // 返回等待状态
}

uint32_t sDRV_ZDTMotor::get_pulse_value()
{
    receive_data(_7_BYTES);   // 先准备接收：地址+命令+符号+4字节脉冲数+校验码
    send_data(CMD_GET_PULSE_VALUE);
    return RT_WAITING;   // 返回等待状态
}

uint32_t sDRV_ZDTMotor::get_motor_position()
{
    receive_data(_8_BYTES);   // 先准备接收：实际返回8字节
    send_data(CMD_GET_MOTOR_POSITION);
    return RT_WAITING;   // 返回等待状态
}

uint16_t sDRV_ZDTMotor::get_motor_position_err()
{
    receive_data(_7_BYTES);   // 先准备接收：地址+命令+符号+4字节误差+校验码
    send_data(CMD_GET_MOTOR_POSITION_ERR);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::get_en_status()
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收：地址+命令+1字节状态+校验码
    send_data(CMD_GET_EN_STATUS);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::get_protect_watermark()
{
    receive_data(_3_BYTES);   // 先准备接收
    send_data(CMD_GET_PROTECT_WATERMARK);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::get_o_status()
{
    receive_data(_3_BYTES);   // 先准备接收
    send_data(CMD_GET_O_STATUS);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::edit_m_step(uint8_t m_step)
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收
    send_data(CMD_EDIT_M_STEP, m_step);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::edit_uart_addr(uint8_t addr)
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收
    send_data(CMD_EDIT_UART_ADDR, addr);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::ctrl_en(uint8_t enabled)
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收
    // 根据协议：地址 + 0xF3 + 0xAB + 使能状态 + 多机同步标志 + 校验字节
    send_data(CMD_CTRL_EN, 0xAB, enabled, 0x00);   // 0x00表示不启用多机同步
    return RT_WAITING;                             // 返回等待状态
}

uint8_t sDRV_ZDTMotor::ctrl_speed_mode(uint8_t direction, uint8_t speed, uint8_t acceleration)
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收
    // 根据协议：地址 + 0xF6 + 方向 + 速度(2字节) + 加速度 + 多机同步标志 + 校验字节
    // 注意：速度是2字节，需要拆分为高字节和低字节
    uint16_t speed_16 = speed;   // 将8位扩展为16位
    send_data(CMD_CTRL_SPEED_MODE, direction, (speed_16 >> 8) & 0xFF, speed_16 & 0xFF, acceleration, 0x00);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::clear_speed_mode()
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收
    send_data(CMD_CLEAR_SPEED_MODE_P1, CMD_CLEAR_SPEED_MODE_P2);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::save_speed_mode()
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收
    send_data(CMD_SAVE_SPEED_MODE_P1, CMD_SAVE_SPEED_MODE_P2);
    return RT_WAITING;   // 返回等待状态
}

uint8_t sDRV_ZDTMotor::ctrl_pos_mode(uint8_t direction, uint16_t speed, uint8_t acceleration, uint32_t pulse_count,
                                     uint8_t relative_mode)
{
    receive_data(_DEFAULT_BYTES);   // 先准备接收
    // 根据协议：地址 + 0xFD + 方向 + 速度(2字节) + 加速度 + 脉冲数(4字节) + 相对/绝对模式标志 + 多机同步标志 + 校验字节
    // 使用数组方式发送位置模式命令
    unsigned char params[] = {
        direction,
        static_cast<unsigned char>((speed >> 8) & 0xFF),
        static_cast<unsigned char>(speed & 0xFF),   // 速度2字节
        acceleration,
        static_cast<unsigned char>((pulse_count >> 24) & 0xFF),
        static_cast<unsigned char>((pulse_count >> 16) & 0xFF),
        static_cast<unsigned char>((pulse_count >> 8) & 0xFF),
        static_cast<unsigned char>(pulse_count & 0xFF),   // 脉冲数4字节
        relative_mode,                                    // 相对/绝对模式标志
        0x00                                              // 多机同步标志
    };
    send_data_array(CMD_CTRL_POS_MODE, params, sizeof(params));
    return RT_WAITING;   // 返回等待状态
}
