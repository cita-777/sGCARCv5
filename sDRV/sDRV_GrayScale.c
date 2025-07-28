#include "sDRV_GrayScale.h"
#include "sBSP_I2C.h"

/**
 * sDRV_GrayScale.c
 * 感为智能8路灰度传感器驱动实现
 * 基于iic_track_example移植到sGCARC项目的sDRV层
 * 
 * v1.0 2025.01.27
 * 移植自iic_track_example，适配sBSP_I2C接口
 */

//****************************************接口******************************************************
// 基于sBSP_I2C的底层接口适配

/**
 * @brief 读取单个字节数据
 * @param slave_addr 从设备地址
 * @return 读取到的数据
 */
static uint8_t IIC_ReadByte(uint8_t slave_addr)
{
    uint8_t dat;
    if (sBSP_I2C1M_ReadBytes(slave_addr, &dat, 1) == HAL_OK) {
        return dat;
    }
    return 0;
}

/**
 * @brief 从指定寄存器读取多个字节
 * @param slave_addr 从设备地址
 * @param reg_addr 寄存器地址
 * @param result 结果缓冲区
 * @param len 读取长度
 * @return 1表示成功，0表示失败
 */
static uint8_t IIC_ReadBytes(uint8_t slave_addr, uint8_t reg_addr, uint8_t* result, uint8_t len)
{
    return (sBSP_I2C1M_MemReadBytes(slave_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, result, len) == HAL_OK) ? 1 : 0;
}

/**
 * @brief 向指定寄存器写入单个字节
 * @param slave_addr 从设备地址
 * @param reg_addr 寄存器地址
 * @param data 要写入的数据
 * @return 1表示成功，0表示失败
 */
static uint8_t IIC_WriteByte(uint8_t slave_addr, uint8_t reg_addr, uint8_t data)
{
    uint8_t dat[2] = {reg_addr, data};
    return (sBSP_I2C1M_SendBytes(slave_addr, dat, 2) == HAL_OK) ? 1 : 0;
}

/**
 * @brief 向指定寄存器写入多个字节
 * @param slave_addr 从设备地址
 * @param reg_addr 寄存器地址
 * @param data 要写入的数据
 * @param len 写入长度
 * @return 1表示成功，0表示失败
 */
static uint8_t IIC_WriteBytes(uint8_t slave_addr, uint8_t reg_addr, uint8_t* data, uint8_t len)
{
    return (sBSP_I2C1M_MemSendBytes(slave_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, data, len) == HAL_OK) ? 1 : 0;
}

//****************************************驱动函数实现******************************************************

/**
 * @brief 初始化灰度传感器
 * @return 0表示成功，-1表示失败
 */
int8_t sDRV_GrayScale_Init(void)
{
    // I2C已经在BSP层初始化，这里只需要检查传感器是否响应
    if (sDRV_GrayScale_Ping() == 0) {
        return 0;  // 初始化成功
    }
    return -1;  // 初始化失败
}

/**
 * @brief Ping传感器，检查是否在线
 * @return 0表示在线，1表示离线
 */
int8_t sDRV_GrayScale_Ping(void)
{
    uint8_t dat;
    if (IIC_ReadBytes(GW_GRAY_ADDR_DEF << 1, GW_GRAY_PING, &dat, 1)) {
        if (dat == GW_GRAY_PING_OK) {
            return 0;  // Ping成功
        }
    }
    return 1;  // Ping失败
}

/**
 * @brief 获取数字模式数据
 * @return 8位数字数据
 */
uint8_t sDRV_GrayScale_GetDigital(void)
{
    uint8_t dat;
    if (IIC_ReadBytes(GW_GRAY_ADDR_DEF << 1, GW_GRAY_DIGITAL_MODE, &dat, 1)) {
        return dat;
    }
    return 0;
}

/**
 * @brief 获取模拟数据
 * @param result 结果缓冲区
 * @param len 读取长度
 * @return 0表示成功，-1表示失败
 */
int8_t sDRV_GrayScale_GetAnalog(uint8_t* result, uint8_t len)
{
    if (IIC_ReadBytes(GW_GRAY_ADDR_DEF << 1, GW_GRAY_ANALOG_BASE_, result, len)) {
        return 0;  // 成功
    }
    return -1;  // 失败
}

/**
 * @brief 获取单个通道的模拟数据
 * @param channel 通道号(1-8)
 * @return 模拟数据值
 */
uint8_t sDRV_GrayScale_GetSingleAnalog(uint8_t channel)
{
    uint8_t dat;
    if (IIC_ReadBytes(GW_GRAY_ADDR_DEF << 1, GW_GRAY_ANALOG(channel), &dat, 1)) {
        return dat;
    }
    return 0;
}

/**
 * @brief 设置归一化通道
 * @param normalize_channel 归一化通道掩码
 * @return 0表示成功，-1表示失败
 */
int8_t sDRV_GrayScale_SetNormalize(uint8_t normalize_channel)
{
    if (IIC_WriteBytes(GW_GRAY_ADDR_DEF << 1, GW_GRAY_ANALOG_NORMALIZE, &normalize_channel, 1)) {
        return 0;  // 成功
    }
    return -1;  // 失败
}

/**
 * @brief 获取偏移值
 * @return 偏移值
 */
uint16_t sDRV_GrayScale_GetOffset(void)
{
    uint8_t dat[2] = {0};
    if (IIC_ReadBytes(GW_GRAY_ADDR_DEF << 1, Offset, dat, 2)) {
        return (uint16_t)dat[0] | ((uint16_t)dat[1] << 8);
    }
    return 0;
}
