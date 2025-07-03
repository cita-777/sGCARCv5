
/*----------------------------------include-----------------------------------*/
#include "sDRV_YaHaboom_voice.h"
#include "FreeRTOS.h"
#include "sDBG_Debug.h"
#include "task.h"

/*-----------------------------------macro------------------------------------*/
#define VOICE_TIMEOUT_MS 1000   // I2C通信超时时间

/*----------------------------------typedef-----------------------------------*/

/*----------------------------------variable----------------------------------*/
static bool g_voice_initialized = false;   // 语音模块初始化标志

/*-------------------------------------os-------------------------------------*/

/*----------------------------------function----------------------------------*/

/**
 * @brief 语音模块初始化
 * @return 0: 成功, -1: 失败
 */
int8_t sDRV_YaHaboom_Voice_Init(void)
{
    sBSP_UART_Debug_Printf("YaHaboom Voice: 开始初始化，I2C地址: 0x%02X\n", VOICE_ADDR << 1);

    // 检查I2C设备是否就绪
    if (!sBSP_I2C1M_DevIsReady(VOICE_ADDR << 1))
    {
        sBSP_UART_Debug_Printf("YaHaboom Voice: 设备未就绪 (地址: 0x%02X)\n", VOICE_ADDR << 1);
        return -1;
    }

    // 延时等待设备稳定
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // 先不发送初始化播报，避免模块一直返回初始化状态
    // 如果需要初始化提示音，可以在任务启动后单独调用

    g_voice_initialized = true;
    sBSP_UART_Debug_Printf("YaHaboom Voice: 初始化成功\n");
    return 0;
}

/**
 * @brief 播报指定内容
 * @param data 要播报的内容代码
 * @return 0: 成功, -1: 失败
 */
int8_t sDRV_YaHaboom_Voice_Broadcast(uint8_t data)
{
    uint8_t tx_data[2] = {WRITE_REGISTER, data};

    // 发送写寄存器命令和数据
    HAL_StatusTypeDef status = sBSP_I2C1M_SendBytes(VOICE_ADDR << 1, tx_data, 2);

    if (status != HAL_OK)
    {
        sBSP_UART_Debug_Printf("YaHaboom Voice: 播报失败, 错误码: %d\n", status);
        return -1;
    }

    sBSP_UART_Debug_Printf("YaHaboom Voice: 播报内容: 0x%02X\n", data);
    return 0;
}

/**
 * @brief 读取语音识别数据
 * @return 识别结果
 */
voice_result_t sDRV_YaHaboom_Voice_ReadData(void)
{
    uint8_t rx_data = 0;

    // 使用内存读取方式，这与原始例程的i2cRead方法相对应
    // 参数：设备地址，寄存器地址，地址大小(8位)，读取的数据
    rx_data = sBSP_I2C1M_MemReadByte(VOICE_ADDR << 1, READ_REGISTER, 0x00000001U);

    // 临时显示所有读取到的数据，用于调试
    static uint8_t  last_rx_data = 0xFF;
    static uint32_t same_count   = 0;

    if (rx_data == last_rx_data)
    {
        same_count++;
        // 每50次相同数据只打印一次，避免刷屏
        if (same_count % 50 == 1)
        {
            sBSP_UART_Debug_Printf("YaHaboom Voice: 持续读取到: 0x%02X (已重复%lu次)\n", rx_data, same_count);
        }
    }
    else
    {
        // 数据变化了，立即打印
        sBSP_UART_Debug_Printf("YaHaboom Voice: 数据变化: 0x%02X -> 0x%02X\n", last_rx_data, rx_data);
        last_rx_data = rx_data;
        same_count   = 1;
    }

    // 解析识别结果
    voice_result_t        result      = VOICE_RESULT_NONE;
    static voice_result_t last_result = VOICE_RESULT_NONE;   // 记录上次识别结果

    switch (rx_data)
    {
    case VOICE_RECOGNIZE_RED: result = VOICE_RESULT_RED; break;
    case VOICE_RECOGNIZE_BLUE: result = VOICE_RESULT_BLUE; break;
    case VOICE_RECOGNIZE_GREEN: result = VOICE_RESULT_GREEN; break;
    case VOICE_RECOGNIZE_YELLOW: result = VOICE_RESULT_YELLOW; break;
    case VOICE_INIT: result = VOICE_RESULT_INIT; break;
    default:
        // 显示所有未知数据，包括0x00和0xFF
        if (rx_data != last_rx_data || same_count == 1)
        {
            sBSP_UART_Debug_Printf("YaHaboom Voice: 未知数据: 0x%02X\n", rx_data);
        }
        break;
    }

    // 防止重复播报：只有当识别结果发生变化时才返回新结果并打印调试信息
    if (result != VOICE_RESULT_NONE && result != last_result)
    {
        last_result = result;

        // 只在第一次识别时打印调试信息
        switch (result)
        {
        case VOICE_RESULT_RED: sBSP_UART_Debug_Printf("YaHaboom Voice: 识别到红色\n"); break;
        case VOICE_RESULT_BLUE: sBSP_UART_Debug_Printf("YaHaboom Voice: 识别到蓝色\n"); break;
        case VOICE_RESULT_GREEN: sBSP_UART_Debug_Printf("YaHaboom Voice: 识别到绿色\n"); break;
        case VOICE_RESULT_YELLOW: sBSP_UART_Debug_Printf("YaHaboom Voice: 识别到黄色\n"); break;
        case VOICE_RESULT_INIT: sBSP_UART_Debug_Printf("YaHaboom Voice: 初始化完成\n"); break;
        default: break;
        }

        return result;
    }
    else if (result == VOICE_RESULT_NONE)
    {
        // 当前没有识别到任何结果，清空上次结果
        last_result = VOICE_RESULT_NONE;
    }

    return VOICE_RESULT_NONE;
}

/**
 * @brief 语音识别任务处理函数
 * 这个函数现在主要用于简单的状态检查，实际处理在任务中进行
 */
void sDRV_YaHaboom_Voice_Handler(void)
{
    if (!g_voice_initialized)
    {
        return;
    }

    // 简单的状态维护，具体处理逻辑在任务中完成
    // 这里可以添加一些状态检查或错误处理逻辑
}

/*------------------------------------test------------------------------------*/
