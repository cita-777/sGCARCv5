/*----------------------------------include-----------------------------------*/
#include "sDRV_HC_SR04.h"
#include "FreeRTOS.h"
#include "main.h"         // 包含dwt对象的定义
#include "sDBG_Debug.h"   // 包含调试输出函数

/*-----------------------------------macro------------------------------------*/
#define TRIG_Pin GPIO_PIN_10
#define TRIG_GPIO_Port GPIOC
#define ECHO_Pin GPIO_PIN_2
#define ECHO_GPIO_Port GPIOD

/*----------------------------------typedef-----------------------------------*/
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    // TRIG引脚配置为输出
    GPIO_InitStruct.Pin   = TRIG_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;   // 添加下拉电阻
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TRIG_GPIO_Port, &GPIO_InitStruct);

    // ECHO引脚配置为输入
    GPIO_InitStruct.Pin   = ECHO_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;   // 添加下拉电阻，避免悬空
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ECHO_GPIO_Port, &GPIO_InitStruct);

    // 确保TRIG引脚初始为低电平
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
}
void sDRV_HC_SR04_Init(void)
{
    GPIO_Init();
}

uint32_t sDRV_HC_SR04_Measure_Distance(void)
{
    uint32_t time_us;
    uint32_t distance;
    uint32_t timeout_counter;

    // 发送触发信号：10us高电平脉冲
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    dwt.delay_us(10);   // HC-SR04需要至少10us的触发脉冲
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

    // 等待Echo引脚变为高电平，添加超时保护
    timeout_counter = 0;
    while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET)
    {
        dwt.delay_us(1);
        timeout_counter++;
        if (timeout_counter > 30000)   // 超时30ms
        {
            dbg_printf("等待Echo高电平超时\n");
            return 0;
        }
    }

    // Echo变为高电平，开始DWT测量
    dwt.start();

    // 等待Echo引脚变为低电平，添加超时保护
    timeout_counter = 0;
    while (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET)
    {
        dwt.delay_us(1);
        timeout_counter++;
        if (timeout_counter > 30000)   // 超时30ms
        {
            dbg_printf("等待Echo低电平超时\n");
            dwt.end();
            return 0;
        }
    }

    // Echo变为低电平，结束DWT测量
    dwt.end();

    // 获取测量时间（微秒）
    time_us = dwt.get_us();

    // 添加有效性检查，防止异常值
    if (time_us == 0 || time_us > 30000)   // 超过30ms认为是异常值
    {
        dbg_printf("时间异常: %lu us\n", time_us);
        return 0;   // 异常值返回0
    }

    // 计算距离: 声速340m/s = 0.034cm/us，往返需要除以2
    // distance = (time_us * 0.034) / 2 = time_us * 0.017
    // 为了避免浮点运算: distance = (time_us * 17) / 1000
    distance = (time_us * 17) / 1000;

    // 距离合理性检查（HC-SR04有效测距范围2cm-400cm）
    if (distance < 2 || distance > 400)
    {
        dbg_printf("距离超出范围: %lu cm\n", distance);
        return 0;   // 超出有效范围返回0
    }

    return distance;   // 返回测量结果
}