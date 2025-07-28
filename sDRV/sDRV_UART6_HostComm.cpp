#include "sDRV_UART6_HostComm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "sBSP_UART.h"
}

// 静态实例指针定义
sDRV_UART6_HostComm* sDRV_UART6_HostComm::instance = nullptr;

sDRV_UART6_HostComm::sDRV_UART6_HostComm(uint32_t baud_rate, uint32_t timeout_ms)
    : baud_rate(baud_rate)
    , timeout_ms(timeout_ms)
    , wheel_speed_callback(nullptr)
    , stepper_pos_callback(nullptr)
    , combined_data_callback(nullptr)
    , error_callback(nullptr)
{
    // 设置静态实例指针
    instance = this;

    // 初始化解析器
    reset_parser();

    // 初始化统计信息
    memset(&stats, 0, sizeof(stats));

    // 初始化数据结构
    memset(&current_wheel_data, 0, sizeof(current_wheel_data));
    memset(&current_stepper_data, 0, sizeof(current_stepper_data));
    memset(&current_combined_data, 0, sizeof(current_combined_data));
}

sDRV_UART6_HostComm::~sDRV_UART6_HostComm()
{
    stop_receive();
    instance = nullptr;
}

int sDRV_UART6_HostComm::init()
{
    // 初始化UART6
    if (sBSP_UART_Top_Init(baud_rate) != 0)
    {
        dbg_printf("[ERROR]UART6_HostComm: UART6初始化失败\n");
        return -1;
    }

    dbg_printf("[INFO]UART6_HostComm: UART6初始化成功，波特率=%u\n", (unsigned int)baud_rate);
    return 0;
}

int sDRV_UART6_HostComm::start_receive()
{
    // 设置UART接收回调
    sBSP_UART_Top_RecvBegin(uart_recv_callback);
    dbg_printf("[INFO]UART6_HostComm: 开始接收数据\n");
    return 0;
}

void sDRV_UART6_HostComm::stop_receive()
{
    // 注意：BSP层没有提供停止接收的函数，这里只是重置解析器
    reset_parser();
    dbg_printf("[INFO]UART6_HostComm: 停止接收数据\n");
}

// 静态回调函数实现
void sDRV_UART6_HostComm::uart_recv_callback(char* data, uint16_t size)
{
    if (instance != nullptr)
    {
        instance->process_received_data(data, size);
    }

    // 重新启动接收
    sBSP_UART_Top_RecvBegin(uart_recv_callback);
}

void sDRV_UART6_HostComm::process_received_data(char* data, uint16_t size)
{
    dbg_printf("[DEBUG]UART6_HostComm: 收到数据 (%d字节): %.*s\n", size, size, data);

    // 逐字节处理数据
    for (uint16_t i = 0; i < size; i++)
    {
        process_byte((uint8_t)data[i]);
    }
}

void sDRV_UART6_HostComm::process_byte(uint8_t byte)
{
    uint32_t current_time = get_current_time_ms();

    // 检查超时
    if (parser.state != UART6_PARSE_IDLE && (current_time - parser.last_byte_time) > timeout_ms)
    {
        handle_parse_error(UART6_ERROR_TIMEOUT);
        reset_parser();
    }

    parser.last_byte_time = current_time;

    switch (parser.state)
    {
    case UART6_PARSE_IDLE:
        if (byte == UART6_PACKET_START_CHAR)
        {
            // 开始新的数据包
            parser.buffer_index                           = 0;
            parser.packet.raw_data[parser.buffer_index++] = (char)byte;
            parser.state                                  = UART6_PARSE_CONTENT;
        }
        break;

    case UART6_PARSE_CONTENT:
        if (parser.buffer_index < UART6_MAX_PACKET_SIZE - 1)
        {
            parser.packet.raw_data[parser.buffer_index++] = (char)byte;

            if (byte == UART6_PACKET_END_CHAR)
            {
                // 数据包结束
                parser.packet.raw_data[parser.buffer_index] = '\0';
                parser.state                                = UART6_PARSE_COMPLETE;

                // 解析文本数据包
                uart6_error_t result = parse_text_packet(parser.packet.raw_data, parser.buffer_index);
                if (result == UART6_OK)
                {
                    stats.valid_packets++;
                }
                else
                {
                    stats.error_packets++;
                    handle_parse_error(result);
                }

                stats.total_packets++;
                stats.last_packet_time = current_time;
                reset_parser();
            }
        }
        else
        {
            // 缓冲区溢出
            handle_parse_error(UART6_ERROR_BUFFER_FULL);
            reset_parser();
        }
        break;

    default: reset_parser(); break;
    }
}

uart6_error_t sDRV_UART6_HostComm::parse_text_packet(const char* text_data, uint16_t length)
{
    dbg_printf("[DEBUG]UART6_HostComm: 解析文本数据包: %s\n", text_data);

    // 复制原始数据
    strncpy(parser.packet.raw_data, text_data, UART6_MAX_PACKET_SIZE - 1);
    parser.packet.raw_data[UART6_MAX_PACKET_SIZE - 1] = '\0';

    // 解析键值对
    uart6_error_t result = parse_key_value_pairs(&parser.packet);
    if (result != UART6_OK)
    {
        return result;
    }

    // 提取数据
    result = extract_data_from_pairs(&parser.packet);
    return result;
}

uart6_error_t sDRV_UART6_HostComm::parse_key_value_pairs(uart6_text_packet_t* packet)
{
    packet->pair_count = 0;

    // 跳过开始的 '{'
    char* content = packet->raw_data + 1;

    // 找到结束的 '}'
    char* end = strrchr(content, UART6_PACKET_END_CHAR);
    if (end == nullptr)
    {
        return UART6_ERROR_INVALID_FORMAT;
    }
    *end = '\0';   // 临时终止字符串

    // 分割键值对
    char* pair_token = strtok(content, ",");
    while (pair_token != nullptr && packet->pair_count < UART6_MAX_PAIRS)
    {
        // 分割键和值
        char* colon = strchr(pair_token, UART6_KEY_VALUE_SEP);
        if (colon == nullptr)
        {
            return UART6_ERROR_INVALID_FORMAT;
        }

        *colon      = '\0';   // 分割键和值
        char* key   = pair_token;
        char* value = colon + 1;

        // 去除空格
        while (*key == ' ') key++;
        while (*value == ' ') value++;

        // 复制键值对
        strncpy(packet->pairs[packet->pair_count].key, key, UART6_MAX_KEY_LENGTH - 1);
        packet->pairs[packet->pair_count].key[UART6_MAX_KEY_LENGTH - 1] = '\0';

        strncpy(packet->pairs[packet->pair_count].value, value, UART6_MAX_VALUE_LENGTH - 1);
        packet->pairs[packet->pair_count].value[UART6_MAX_VALUE_LENGTH - 1] = '\0';

        packet->pair_count++;
        pair_token = strtok(nullptr, ",");
    }

    *end = UART6_PACKET_END_CHAR;   // 恢复字符
    return UART6_OK;
}

uart6_error_t sDRV_UART6_HostComm::extract_data_from_pairs(const uart6_text_packet_t* packet)
{
    // 获取命令类型
    const char* cmd_str = find_value_by_key(packet, UART6_KEY_CMD);
    if (cmd_str == nullptr)
    {
        return UART6_ERROR_MISSING_KEY;
    }

    uart6_cmd_type_t cmd = get_command_type(cmd_str);
    if (cmd == UART6_CMD_UNKNOWN)
    {
        return UART6_ERROR_INVALID_CMD;
    }

    uint32_t current_time = get_current_time_ms();

    // 根据命令类型处理数据
    switch (cmd)
    {
    case UART6_CMD_WHEEL_SPEED:
    {
        const char* lw_str = find_value_by_key(packet, UART6_KEY_LEFT_WHEEL);
        const char* rw_str = find_value_by_key(packet, UART6_KEY_RIGHT_WHEEL);

        if (lw_str && rw_str)
        {
            current_wheel_data.left_wheel_speed  = parse_float_value(lw_str);
            current_wheel_data.right_wheel_speed = parse_float_value(rw_str);
            current_wheel_data.timestamp         = current_time;
            current_wheel_data.valid             = 1;

            dbg_printf("[INFO]UART6_HostComm: 轮速数据 - 左:%.2f, 右:%.2f\n",
                       current_wheel_data.left_wheel_speed,
                       current_wheel_data.right_wheel_speed);

            if (wheel_speed_callback != nullptr)
            {
                wheel_speed_callback(&current_wheel_data);
            }
        }
        break;
    }

    case UART6_CMD_STEPPER_POS:
    {
        const char* s1_str = find_value_by_key(packet, UART6_KEY_STEPPER1);
        const char* s2_str = find_value_by_key(packet, UART6_KEY_STEPPER2);

        if (s1_str && s2_str)
        {
            current_stepper_data.stepper1_position = parse_int32_value(s1_str);
            current_stepper_data.stepper2_position = parse_int32_value(s2_str);
            current_stepper_data.timestamp         = current_time;
            current_stepper_data.valid             = 1;

            dbg_printf("[INFO]UART6_HostComm: 步进电机位置 - 电机1:%ld, 电机2:%ld\n",
                       current_stepper_data.stepper1_position,
                       current_stepper_data.stepper2_position);

            if (stepper_pos_callback != nullptr)
            {
                stepper_pos_callback(&current_stepper_data);
            }
        }
        break;
    }

    case UART6_CMD_COMBINED_DATA:
    {
        const char* lw_str = find_value_by_key(packet, UART6_KEY_LEFT_WHEEL);
        const char* rw_str = find_value_by_key(packet, UART6_KEY_RIGHT_WHEEL);
        const char* s1_str = find_value_by_key(packet, UART6_KEY_STEPPER1);
        const char* s2_str = find_value_by_key(packet, UART6_KEY_STEPPER2);

        if (lw_str && rw_str && s1_str && s2_str)
        {
            // 更新轮速数据
            current_combined_data.wheel_data.left_wheel_speed  = parse_float_value(lw_str);
            current_combined_data.wheel_data.right_wheel_speed = parse_float_value(rw_str);
            current_combined_data.wheel_data.timestamp         = current_time;
            current_combined_data.wheel_data.valid             = 1;

            // 更新步进电机数据
            current_combined_data.stepper_data.stepper1_position = parse_int32_value(s1_str);
            current_combined_data.stepper_data.stepper2_position = parse_int32_value(s2_str);
            current_combined_data.stepper_data.timestamp         = current_time;
            current_combined_data.stepper_data.valid             = 1;

            // 更新组合数据
            current_combined_data.timestamp = current_time;
            current_combined_data.valid     = 1;

            // 同时更新单独的数据结构
            current_wheel_data   = current_combined_data.wheel_data;
            current_stepper_data = current_combined_data.stepper_data;

            dbg_printf("[INFO]UART6_HostComm: 组合数据 - 轮速[左:%.2f,右:%.2f] 步进电机[1:%ld,2:%ld]\n",
                       current_combined_data.wheel_data.left_wheel_speed,
                       current_combined_data.wheel_data.right_wheel_speed,
                       current_combined_data.stepper_data.stepper1_position,
                       current_combined_data.stepper_data.stepper2_position);

            if (combined_data_callback != nullptr)
            {
                combined_data_callback(&current_combined_data);
            }
        }
        break;
    }

    case UART6_CMD_HEARTBEAT: dbg_printf("[INFO]UART6_HostComm: 收到心跳包\n"); break;

    case UART6_CMD_STATUS_REQ:
        // 发送状态响应
        send_heartbeat();
        break;

    case UART6_CMD_RESET:
        dbg_printf("[INFO]UART6_HostComm: 收到复位命令\n");
        reset_statistics();
        break;

    default: return UART6_ERROR_INVALID_CMD;
    }

    return UART6_OK;
}

// 辅助方法实现
uart6_cmd_type_t sDRV_UART6_HostComm::get_command_type(const char* cmd_str)
{
    if (strcmp(cmd_str, UART6_CMD_WHEEL_STR) == 0)
    {
        return UART6_CMD_WHEEL_SPEED;
    }
    else if (strcmp(cmd_str, UART6_CMD_STEPPER_STR) == 0)
    {
        return UART6_CMD_STEPPER_POS;
    }
    else if (strcmp(cmd_str, UART6_CMD_COMBINED_STR) == 0)
    {
        return UART6_CMD_COMBINED_DATA;
    }
    else if (strcmp(cmd_str, UART6_CMD_HEARTBEAT_STR) == 0)
    {
        return UART6_CMD_HEARTBEAT;
    }
    else if (strcmp(cmd_str, UART6_CMD_STATUS_STR) == 0)
    {
        return UART6_CMD_STATUS_REQ;
    }
    else if (strcmp(cmd_str, UART6_CMD_RESET_STR) == 0)
    {
        return UART6_CMD_RESET;
    }
    return UART6_CMD_UNKNOWN;
}

const char* sDRV_UART6_HostComm::find_value_by_key(const uart6_text_packet_t* packet, const char* key)
{
    for (uint8_t i = 0; i < packet->pair_count; i++)
    {
        if (strcmp(packet->pairs[i].key, key) == 0)
        {
            return packet->pairs[i].value;
        }
    }
    return nullptr;
}

float sDRV_UART6_HostComm::parse_float_value(const char* value_str)
{
    return (float)atof(value_str);
}

int32_t sDRV_UART6_HostComm::parse_int32_value(const char* value_str)
{
    return (int32_t)atol(value_str);
}

void sDRV_UART6_HostComm::reset_parser()
{
    parser.state          = UART6_PARSE_IDLE;
    parser.buffer_index   = 0;
    parser.last_byte_time = get_current_time_ms();
    memset(&parser.packet, 0, sizeof(parser.packet));
}

void sDRV_UART6_HostComm::handle_parse_error(uart6_error_t error)
{
    parser.error_count++;

    const char* error_msg = "Unknown error";
    switch (error)
    {
    case UART6_ERROR_INVALID_FORMAT: error_msg = "Invalid format"; break;
    case UART6_ERROR_INVALID_CMD: error_msg = "Invalid command"; break;
    case UART6_ERROR_MISSING_KEY: error_msg = "Missing key"; break;
    case UART6_ERROR_INVALID_VALUE: error_msg = "Invalid value"; break;
    case UART6_ERROR_TIMEOUT:
        error_msg = "Timeout";
        stats.timeout_errors++;
        break;
    case UART6_ERROR_BUFFER_FULL: error_msg = "Buffer full"; break;
    case UART6_ERROR_PARSE_ERROR: error_msg = "Parse error"; break;
    default: break;
    }

    dbg_printf("[ERROR]UART6_HostComm: %s\n", error_msg);

    // 调用错误回调函数
    if (error_callback != nullptr)
    {
        error_callback(error, error_msg);
    }
}

uint32_t sDRV_UART6_HostComm::get_current_time_ms() const
{
    return HAL_GetTick();
}

// 公共方法实现
int sDRV_UART6_HostComm::send_response(uart6_cmd_type_t cmd, const uint8_t* payload, uint8_t length)
{
    // 构建文本响应
    char response[UART6_MAX_PACKET_SIZE];
    int  len = 0;

    // 开始字符
    response[len++] = UART6_PACKET_START_CHAR;

    // 添加命令
    const char* cmd_str = "";
    switch (cmd)
    {
    case UART6_CMD_HEARTBEAT: cmd_str = UART6_CMD_HEARTBEAT_STR; break;
    case UART6_CMD_STATUS_REQ: cmd_str = UART6_CMD_STATUS_STR; break;
    default: cmd_str = "response"; break;
    }

    len += snprintf(response + len, sizeof(response) - len - 1, "cmd:%s", cmd_str);

    // 添加时间戳
    if (len < sizeof(response) - 20)
    {
        len += snprintf(response + len, sizeof(response) - len - 1, ",ts:%u", (unsigned int)get_current_time_ms());
    }

    // 结束字符
    response[len++] = UART6_PACKET_END_CHAR;
    response[len]   = '\0';

    // 发送数据
    sBSP_UART_Top_SendBytes((uint8_t*)response, len);

    dbg_printf("[INFO]UART6_HostComm: 发送响应: %s\n", response);

    return 0;
}

int sDRV_UART6_HostComm::send_heartbeat()
{
    return send_response(UART6_CMD_HEARTBEAT, nullptr, 0);
}

const uart6_wheel_speed_t* sDRV_UART6_HostComm::get_wheel_speed_data() const
{
    return &current_wheel_data;
}

const uart6_stepper_pos_t* sDRV_UART6_HostComm::get_stepper_pos_data() const
{
    return &current_stepper_data;
}

const uart6_combined_data_t* sDRV_UART6_HostComm::get_combined_data() const
{
    return &current_combined_data;
}

const uart6_statistics_t* sDRV_UART6_HostComm::get_statistics() const
{
    return &stats;
}

void sDRV_UART6_HostComm::reset_statistics()
{
    memset(&stats, 0, sizeof(stats));
    dbg_printf("[INFO]UART6_HostComm: 统计信息已重置\n");
}

void sDRV_UART6_HostComm::set_wheel_speed_callback(void (*callback)(const uart6_wheel_speed_t* data))
{
    wheel_speed_callback = callback;
}

void sDRV_UART6_HostComm::set_stepper_pos_callback(void (*callback)(const uart6_stepper_pos_t* data))
{
    stepper_pos_callback = callback;
}

void sDRV_UART6_HostComm::set_combined_data_callback(void (*callback)(const uart6_combined_data_t* data))
{
    combined_data_callback = callback;
}

void sDRV_UART6_HostComm::set_error_callback(void (*callback)(uart6_error_t error, const char* message))
{
    error_callback = callback;
}

bool sDRV_UART6_HostComm::is_data_valid(uint32_t max_age_ms) const
{
    uint32_t current_time = get_current_time_ms();

    // 检查轮速数据是否有效
    bool wheel_valid = current_wheel_data.valid && (current_time - current_wheel_data.timestamp) <= max_age_ms;

    // 检查步进电机数据是否有效
    bool stepper_valid = current_stepper_data.valid && (current_time - current_stepper_data.timestamp) <= max_age_ms;

    return wheel_valid || stepper_valid;
}

void sDRV_UART6_HostComm::print_debug_info() const
{
    uint32_t current_time = get_current_time_ms();

    dbg_printf("=== UART6 Host Communication Debug Info ===\n");
    dbg_printf("统计信息:\n");
    dbg_printf("  总包数: %u\n", (unsigned int)stats.total_packets);
    dbg_printf("  有效包数: %u\n", (unsigned int)stats.valid_packets);
    dbg_printf("  错误包数: %u\n", (unsigned int)stats.error_packets);
    dbg_printf("  超时错误: %u\n", (unsigned int)stats.timeout_errors);
    dbg_printf("  最后包时间: %u ms前\n", (unsigned int)(current_time - stats.last_packet_time));

    dbg_printf("轮速数据:\n");
    dbg_printf("  左轮速度: %.2f\n", current_wheel_data.left_wheel_speed);
    dbg_printf("  右轮速度: %.2f\n", current_wheel_data.right_wheel_speed);
    dbg_printf("  数据年龄: %u ms\n", (unsigned int)(current_time - current_wheel_data.timestamp));
    dbg_printf("  数据有效: %s\n", current_wheel_data.valid ? "是" : "否");

    dbg_printf("步进电机数据:\n");
    dbg_printf("  电机1位置: %ld\n", current_stepper_data.stepper1_position);
    dbg_printf("  电机2位置: %ld\n", current_stepper_data.stepper2_position);
    dbg_printf("  数据年龄: %u ms\n", (unsigned int)(current_time - current_stepper_data.timestamp));
    dbg_printf("  数据有效: %s\n", current_stepper_data.valid ? "是" : "否");

    dbg_printf("解析器状态:\n");
    dbg_printf("  当前状态: %d\n", parser.state);
    dbg_printf("  错误计数: %u\n", (unsigned int)parser.error_count);
    dbg_printf("============================================\n");
}
