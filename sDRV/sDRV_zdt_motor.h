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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sBSP_UART.h"
#include "sDBG_Debug.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

// return status
#define RT_WARNING 0
#define RT_OK 1
#define RT_SUCCESS 2
#define RT_ERROR -1
#define RT_WAITING -2

typedef enum
{
    _DEFAULT_BYTES = 4,   // 标准控制命令响应：地址+命令+状态+校验码
    _3_BYTES       = 3,   // 3字节响应：地址+数据+校验码
    _4_BYTES       = 4,   // 4字节响应：地址+命令+状态+校验码
    _5_BYTES       = 5,   // 5字节响应：地址+命令+2字节数据+校验码
    _6_BYTES       = 6,   // 6字节响应：地址+命令+3字节数据+校验码
    _7_BYTES       = 7,   // 7字节响应：地址+命令+5字节数据+校验码
    _8_BYTES       = 8,   // 8字节响应：地址+命令+6字节数据+校验码
} byte_num_t;

// commands, DO NOT modify it !!
typedef enum
{
    // __GET_CMDS (根据协议文档修正)
    CMD_GET_ENCODER_VALUE      = 0x31,   // 读取经过线性化校准后的编码器值
    CMD_GET_PULSE_VALUE        = 0x32,   // 读取输入脉冲数
    CMD_GET_MOTOR_POSITION     = 0x36,   // 读取电机实时位置
    CMD_GET_MOTOR_POSITION_ERR = 0x37,   // 读取电机位置误差
    CMD_GET_EN_STATUS          = 0x3A,   // 读取电机状态标志位
    CMD_GET_MOTOR_SPEED        = 0x35,   // 读取电机实时转速
    CMD_GET_O_STATUS           = 0x3B,   // 读取回零状态标志位
    CMD_GET_PROTECT_WATERMARK  = 0x3A,   // 读取保护水印（与状态相同）
                                         // __EDIT_CMDS
    CMD_EDIT_M_STEP    = 0x84,
    CMD_EDIT_UART_ADDR = 0xAE,
    // __CTRL_CMDS
    CMD_CTRL_EN         = 0xF3,
    CMD_CTRL_SPEED_MODE = 0xF6,
    CMD_CTRL_POS_MODE   = 0xFD,
    //
    CMD_CANCEL_MOTOR_PROTECT_P1 = 0x0E,
    CMD_CANCEL_MOTOR_PROTECT_P2 = 0x52,
    CMD_CLEAR_SPEED_MODE_P1     = 0xFF,
    CMD_CLEAR_SPEED_MODE_P2     = 0xCA,
    CMD_SAVE_SPEED_MODE_P1      = 0xFF,
    CMD_SAVE_SPEED_MODE_P2      = 0xC8
} cmd_t;

#ifdef __cplusplus
}

class sDRV_ZDTMotor
{
private:
    unsigned char             address;
    unsigned int              baud_rate;
    unsigned char             data_check_byte;   // 默认值 = 0x6B
    static const unsigned int timeout_ms = 100;

    // 接收状态管理
    volatile bool          recv_complete;
    volatile signed int    recv_result;
    volatile unsigned char expected_recv_length;

    // 静态实例指针，用于回调函数
    static sDRV_ZDTMotor* instance;

    // 静态回调函数
    static void uart_recv_callback(char* data, unsigned short size);

    // 处理接收到的数据
    void process_received_data(char* data, unsigned short size);

public:
    // 查询接收状态
    bool       is_recv_complete() const { return recv_complete; }
    signed int get_recv_result() const { return recv_result; }
    /**
     * @brief 初始化ZDT步进电机库 (STM32F405版本)
     *
     * 该构造函数用于初始化ZDT步进电机库，使用UART3 (PB10/PB11)进行通信
     * IMU功能已禁用，UART3现在专用于步进电机
     *
     * @param address 步进电机地址，默认为1
     * @param baud_rate 波特率配置，默认值为115200
     * @param data_check_byte 数据校验字节，默认值为0x6B
     */
    sDRV_ZDTMotor(unsigned char address = 1, unsigned int baud_rate = 115200, unsigned char data_check_byte = 0x6B);

private:
    int init();

    /**
     * @brief （私有函数）发送数据 - 重载版本
     */
    bool send_data(unsigned char cmd);
    bool send_data(unsigned char cmd, unsigned char p1);
    bool send_data(unsigned char cmd, unsigned char p1, unsigned char p2);
    bool send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3);
    bool send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4);
    bool send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4,
                   unsigned char p5);
    bool send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4,
                   unsigned char p5, unsigned char p6);
    bool send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4,
                   unsigned char p5, unsigned char p6, unsigned char p7, unsigned char p8, unsigned char p9);
    bool send_data(unsigned char cmd, unsigned char p1, unsigned char p2, unsigned char p3, unsigned char p4,
                   unsigned char p5, unsigned char p6, unsigned char p7, unsigned char p8, unsigned char p9,
                   unsigned char p10);
    bool send_data_array(unsigned char cmd, unsigned char* params, unsigned char param_count);

    /**
     * @brief （私有函数）接收数据
     */
    signed int receive_data(signed char expected_length);

public:
    /**
     * @brief 解除堵转保护
     */
    unsigned char cancel_motor_protect();

    /**
     * @brief 读取编码器值
     */
    unsigned short get_encoder_value();

    /**
     * @brief 读取输入脉冲数
     */
    unsigned int get_pulse_value();

    /**
     * @brief 读取电机实时位置
     */
    unsigned int get_motor_position();

    /**
     * @brief 读取电机位置误差
     */
    unsigned short get_motor_position_err();

    /**
     * @brief 读取使能状态
     */
    unsigned char get_en_status();

    /**
     * @brief 读取堵转标志
     */
    unsigned char get_protect_watermark();

    /**
     * @brief 读取单圈上电自动回零状态标志
     */
    unsigned char get_o_status();

    /**
     * @brief 修改当前细分步数
     */
    unsigned char edit_m_step(unsigned char m_step);

    /**
     * @brief 修改当前串口通讯地址
     */
    unsigned char edit_uart_addr(unsigned char addr);

    /**
     * @brief 控制闭环电机的使能状态
     */
    unsigned char ctrl_en(unsigned char enabled);

    /**
     * @brief 控制闭环电机的正反转，即速度模式控制
     */
    unsigned char ctrl_speed_mode(unsigned char direction, unsigned char speed, unsigned char acceleration);

    /**
     * @brief 清除闭环电机正反转，即速度模式当前的参数
     */
    unsigned char clear_speed_mode();

    /**
     * @brief 存储闭环电机正反转，即速度模式当前的参数
     */
    unsigned char save_speed_mode();

    /**
     * @brief 控制闭环电机相对运动的角度，即位置模式控制
     * @param direction 方向：0=CW，1=CCW
     * @param speed 速度(RPM)
     * @param acceleration 加速度档位
     * @param pulse_count 脉冲数
     * @param relative_mode 相对/绝对模式：0=相对，1=绝对
     */
    unsigned char ctrl_pos_mode(unsigned char direction, uint16_t speed, unsigned char acceleration,
                                uint32_t pulse_count, unsigned char relative_mode);
};

#endif
