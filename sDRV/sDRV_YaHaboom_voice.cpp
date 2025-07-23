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

    // 根据代码显示中文内容
    const char* content_str = get_voice_code_description(data);

    sBSP_UART_Debug_Printf("YaHaboom Voice: 播报内容: %s (0x%02X)\n", content_str, data);
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
    case VOICE_START_STRAIGHT_WALK: result = VOICE_RESULT_START_STRAIGHT_WALK; break;
    case VOICE_START_DIRECTION_REPORT: result = VOICE_RESULT_START_DIRECTION_REPORT; break;
    case VOICE_LOW_BATTERY_WARNING: result = VOICE_RESULT_LOW_BATTERY_WARNING; break;
    case VOICE_QUERY_BATTERY_STATUS: result = VOICE_RESULT_QUERY_BATTERY_STATUS; break;
    case VOICE_BATTERY_SUFFICIENT: result = VOICE_RESULT_BATTERY_SUFFICIENT; break;
    case VOICE_FALL_DETECTED_ALARM: result = VOICE_RESULT_FALL_DETECTED_ALARM; break;
    case VOICE_OBSTACLE_30CM_WARNING: result = VOICE_RESULT_OBSTACLE_30CM_WARNING; break;
    case VOICE_OBSTACLE_10CM_ALARM: result = VOICE_RESULT_OBSTACLE_10CM_ALARM; break;
    case VOICE_SELF_CHECK_PASS: result = VOICE_RESULT_SELF_CHECK_PASS; break;
    case VOICE_SELF_CHECK_FAIL: result = VOICE_RESULT_SELF_CHECK_FAIL; break;
    case VOICE_FACING_SOUTH: result = VOICE_RESULT_FACING_SOUTH; break;
    case VOICE_FACING_NORTH: result = VOICE_RESULT_FACING_NORTH; break;
    case VOICE_FACING_EAST: result = VOICE_RESULT_FACING_EAST; break;
    case VOICE_FACING_WEST: result = VOICE_RESULT_FACING_WEST; break;
    case VOICE_STOP_DIRECTION_REPORT: result = VOICE_RESULT_STOP_DIRECTION_REPORT; break;
    case VOICE_INIT: result = VOICE_RESULT_INIT; break;
    default:
        // 显示所有未知数据，包括0x00和0xFF
        if (rx_data != last_rx_data || same_count == 1)
        {
            const char* desc = "";
            switch (rx_data)
            {
            case 0x00: desc = " (无数据)"; break;
            case 0xFF: desc = " (默认值)"; break;
            default: desc = " (未知指令)"; break;
            }
            sBSP_UART_Debug_Printf("YaHaboom Voice: 未知数据: 0x%02X%s\n", rx_data, desc);
        }
        break;
    }

    // 防止重复播报：只有当识别结果发生变化时才返回新结果并打印调试信息
    if (result != VOICE_RESULT_NONE && result != last_result)
    {
        last_result = result;
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

/**
 * @brief 播报开启直线行走模式
 */
void sDRV_YaHaboom_Voice_BroadcastStraightWalk(void)
{
    sDRV_YaHaboom_Voice_Broadcast(VOICE_START_STRAIGHT_WALK);
}

/**
 * @brief 播报定时播报当前方向的开启/关闭
 * @param enable true: 开启, false: 关闭
 */
void sDRV_YaHaboom_Voice_BroadcastDirectionReport(bool enable)
{
    if (enable)
    {
        sDRV_YaHaboom_Voice_Broadcast(VOICE_START_DIRECTION_REPORT);
    }
    else
    {
        sDRV_YaHaboom_Voice_Broadcast(VOICE_STOP_DIRECTION_REPORT);
    }
}

/**
 * @brief 播报电池状态
 * @param is_low true: 电量不足, false: 电量充足
 */
void sDRV_YaHaboom_Voice_BroadcastBatteryStatus(bool is_low)
{
    if (is_low)
    {
        sDRV_YaHaboom_Voice_Broadcast(VOICE_LOW_BATTERY_WARNING);
    }
    else
    {
        sDRV_YaHaboom_Voice_Broadcast(VOICE_BATTERY_SUFFICIENT);
    }
}

/**
 * @brief 播报障碍物警告
 * @param distance_cm 障碍物距离(厘米)
 */
void sDRV_YaHaboom_Voice_BroadcastObstacleWarning(uint8_t distance_cm)
{
    if (distance_cm <= 10)
    {
        sDRV_YaHaboom_Voice_Broadcast(VOICE_OBSTACLE_10CM_ALARM);
    }
    else if (distance_cm <= 30)
    {
        sDRV_YaHaboom_Voice_Broadcast(VOICE_OBSTACLE_30CM_WARNING);
    }
}

/**
 * @brief 播报自检结果
 * @param pass true: 自检通过, false: 自检失败
 */
void sDRV_YaHaboom_Voice_BroadcastSelfCheck(bool pass)
{
    if (pass)
    {
        sDRV_YaHaboom_Voice_Broadcast(VOICE_SELF_CHECK_PASS);
    }
    else
    {
        sDRV_YaHaboom_Voice_Broadcast(VOICE_SELF_CHECK_FAIL);
    }
}

/**
 * @brief 播报当前朝向
 * @param direction 方向: 0-南, 1-北, 2-东, 3-西
 */
void sDRV_YaHaboom_Voice_BroadcastDirection(uint8_t direction)
{
    switch (direction)
    {
    case 0: sDRV_YaHaboom_Voice_Broadcast(VOICE_FACING_SOUTH); break;
    case 1: sDRV_YaHaboom_Voice_Broadcast(VOICE_FACING_NORTH); break;
    case 2: sDRV_YaHaboom_Voice_Broadcast(VOICE_FACING_EAST); break;
    case 3: sDRV_YaHaboom_Voice_Broadcast(VOICE_FACING_WEST); break;
    default: break;
    }
}

/**
 * @brief 获取语音代码的中文描述
 * @param code 语音代码
 * @return 中文描述字符串
 */
static const char* get_voice_code_description(uint8_t code)
{
    switch (code)
    {
    case VOICE_START_STRAIGHT_WALK: return "开启直线行走模式";
    case VOICE_START_DIRECTION_REPORT: return "开启定时播报当前方向";
    case VOICE_LOW_BATTERY_WARNING: return "当前电量不足,请及时充电";
    case VOICE_QUERY_BATTERY_STATUS: return "现在电量状况";
    case VOICE_BATTERY_SUFFICIENT: return "当前电量充足";
    case VOICE_FALL_DETECTED_ALARM: return "检测到跌落,开启报警";
    case VOICE_OBSTACLE_30CM_WARNING: return "前方障碍物不足三十厘米,注意安全";
    case VOICE_OBSTACLE_10CM_ALARM: return "前方障碍物不足十厘米,开启报警";
    case VOICE_SELF_CHECK_PASS: return "自检通过,智能导盲助手已启动";
    case VOICE_SELF_CHECK_FAIL: return "自检失败,请进行维修";
    case VOICE_FACING_SOUTH: return "目前朝向南方";
    case VOICE_FACING_NORTH: return "目前朝向北方";
    case VOICE_FACING_EAST: return "目前朝向东方";
    case VOICE_FACING_WEST: return "目前朝向西方";
    case VOICE_STOP_DIRECTION_REPORT: return "关闭定时播报当前方向";
    case VOICE_INIT: return "初始化语音";
    default: return "未知内容";
    }
}

/*------------------------------------test------------------------------------*/
