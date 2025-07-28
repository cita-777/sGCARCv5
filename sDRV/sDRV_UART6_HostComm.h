#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sBSP_UART.h"
#include "sDBG_Debug.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

// 文本协议常量定义
#define UART6_PACKET_START_CHAR '{'   // 数据包开始字符
#define UART6_PACKET_END_CHAR '}'     // 数据包结束字符
#define UART6_KEY_VALUE_SEP ':'       // 键值分隔符
#define UART6_PAIR_SEP ','            // 键值对分隔符
#define UART6_MAX_PACKET_SIZE 128     // 最大数据包大小
#define UART6_MAX_KEY_LENGTH 16       // 最大键长度
#define UART6_MAX_VALUE_LENGTH 32     // 最大值长度
#define UART6_MAX_PAIRS 8             // 最大键值对数量

// 命令类型字符串定义
#define UART6_CMD_WHEEL_STR "wheel"           // 轮速控制命令
#define UART6_CMD_STEPPER_STR "stepper"       // 步进电机位置命令
#define UART6_CMD_COMBINED_STR "combined"     // 组合数据命令
#define UART6_CMD_HEARTBEAT_STR "heartbeat"   // 心跳包
#define UART6_CMD_STATUS_STR "status"         // 状态请求
#define UART6_CMD_RESET_STR "reset"           // 复位命令

// 数据字段键名定义
#define UART6_KEY_CMD "cmd"          // 命令类型
#define UART6_KEY_LEFT_WHEEL "lw"    // 左轮速度
#define UART6_KEY_RIGHT_WHEEL "rw"   // 右轮速度
#define UART6_KEY_STEPPER1 "s1"      // 步进电机1位置
#define UART6_KEY_STEPPER2 "s2"      // 步进电机2位置
#define UART6_KEY_TIMESTAMP "ts"     // 时间戳

// 命令类型枚举
typedef enum
{
    UART6_CMD_UNKNOWN = 0,     // 未知命令
    UART6_CMD_WHEEL_SPEED,     // 轮速控制命令
    UART6_CMD_STEPPER_POS,     // 步进电机位置命令
    UART6_CMD_COMBINED_DATA,   // 组合数据命令
    UART6_CMD_HEARTBEAT,       // 心跳包
    UART6_CMD_STATUS_REQ,      // 状态请求
    UART6_CMD_RESET            // 复位命令
} uart6_cmd_type_t;

// 解析状态定义
typedef enum
{
    UART6_PARSE_IDLE,       // 空闲状态，等待开始字符
    UART6_PARSE_CONTENT,    // 解析数据包内容
    UART6_PARSE_COMPLETE,   // 解析完成
    UART6_PARSE_ERROR       // 解析错误
} uart6_parse_state_t;

// 错误代码定义
typedef enum
{
    UART6_OK = 0,                 // 成功
    UART6_ERROR_INVALID_FORMAT,   // 无效格式
    UART6_ERROR_INVALID_CMD,      // 无效命令
    UART6_ERROR_MISSING_KEY,      // 缺少必需的键
    UART6_ERROR_INVALID_VALUE,    // 无效值
    UART6_ERROR_TIMEOUT,          // 超时
    UART6_ERROR_BUFFER_FULL,      // 缓冲区满
    UART6_ERROR_PARSE_ERROR,      // 解析错误
    UART6_ERROR_UNKNOWN           // 未知错误
} uart6_error_t;

// 轮速数据结构
typedef struct
{
    float    left_wheel_speed;    // 左轮速度 (m/s 或 RPM)
    float    right_wheel_speed;   // 右轮速度 (m/s 或 RPM)
    uint32_t timestamp;           // 时间戳
    uint8_t  valid;               // 数据有效标志
} uart6_wheel_speed_t;

// 步进电机位置数据结构
typedef struct
{
    int32_t  stepper1_position;   // 步进电机1位置
    int32_t  stepper2_position;   // 步进电机2位置
    uint32_t timestamp;           // 时间戳
    uint8_t  valid;               // 数据有效标志
} uart6_stepper_pos_t;

// 组合数据结构
typedef struct
{
    uart6_wheel_speed_t wheel_data;     // 轮速数据
    uart6_stepper_pos_t stepper_data;   // 步进电机数据
    uint32_t            timestamp;      // 时间戳
    uint8_t             valid;          // 数据有效标志
} uart6_combined_data_t;

// 键值对结构
typedef struct
{
    char key[UART6_MAX_KEY_LENGTH];       // 键名
    char value[UART6_MAX_VALUE_LENGTH];   // 值
} uart6_key_value_pair_t;

// 文本数据包结构
typedef struct
{
    char                   raw_data[UART6_MAX_PACKET_SIZE];   // 原始数据
    uart6_key_value_pair_t pairs[UART6_MAX_PAIRS];            // 键值对数组
    uint8_t                pair_count;                        // 键值对数量
    uart6_cmd_type_t       command;                           // 解析出的命令类型
} uart6_text_packet_t;

// 解析器状态结构
typedef struct
{
    uart6_parse_state_t state;            // 当前解析状态
    uart6_text_packet_t packet;           // 当前文本数据包
    uint8_t             buffer_index;     // 缓冲区索引
    uint32_t            last_byte_time;   // 最后接收字节的时间
    uint32_t            error_count;      // 错误计数
} uart6_parser_t;

// 统计信息结构
typedef struct
{
    uint32_t total_packets;      // 总包数
    uint32_t valid_packets;      // 有效包数
    uint32_t error_packets;      // 错误包数
    uint32_t checksum_errors;    // 校验和错误数
    uint32_t timeout_errors;     // 超时错误数
    uint32_t last_packet_time;   // 最后一个包的时间
} uart6_statistics_t;

#ifdef __cplusplus
}

// C++类定义
class sDRV_UART6_HostComm
{
private:
    // 静态实例指针，用于回调函数
    static sDRV_UART6_HostComm* instance;

    // 解析器和统计信息
    uart6_parser_t     parser;
    uart6_statistics_t stats;

    // 数据存储
    uart6_wheel_speed_t   current_wheel_data;
    uart6_stepper_pos_t   current_stepper_data;
    uart6_combined_data_t current_combined_data;

    // 配置参数
    uint32_t baud_rate;
    uint32_t timeout_ms;

    // 回调函数指针
    void (*wheel_speed_callback)(const uart6_wheel_speed_t* data);
    void (*stepper_pos_callback)(const uart6_stepper_pos_t* data);
    void (*combined_data_callback)(const uart6_combined_data_t* data);
    void (*error_callback)(uart6_error_t error, const char* message);

    // 私有方法
    static void      uart_recv_callback(char* data, uint16_t size);
    void             process_received_data(char* data, uint16_t size);
    void             process_byte(uint8_t byte);
    uart6_error_t    parse_text_packet(const char* text_data, uint16_t length);
    uart6_error_t    parse_key_value_pairs(uart6_text_packet_t* packet);
    uart6_error_t    extract_data_from_pairs(const uart6_text_packet_t* packet);
    uart6_cmd_type_t get_command_type(const char* cmd_str);
    const char*      find_value_by_key(const uart6_text_packet_t* packet, const char* key);
    float            parse_float_value(const char* value_str);
    int32_t          parse_int32_value(const char* value_str);
    void             reset_parser();
    void             handle_parse_error(uart6_error_t error);
    uint32_t         get_current_time_ms() const;

public:
    /**
     * @brief 构造函数
     * @param baud_rate 波特率，默认115200
     * @param timeout_ms 超时时间，默认1000ms
     */
    sDRV_UART6_HostComm(uint32_t baud_rate = 115200, uint32_t timeout_ms = 1000);

    /**
     * @brief 析构函数
     */
    ~sDRV_UART6_HostComm();

    /**
     * @brief 初始化UART6通信
     * @return 0成功，-1失败
     */
    int init();

    /**
     * @brief 开始接收数据
     * @return 0成功，-1失败
     */
    int start_receive();

    /**
     * @brief 停止接收数据
     */
    void stop_receive();

    /**
     * @brief 发送响应数据包
     * @param cmd 命令类型
     * @param payload 有效载荷数据
     * @param length 有效载荷长度
     * @return 0成功，-1失败
     */
    int send_response(uart6_cmd_type_t cmd, const uint8_t* payload, uint8_t length);

    /**
     * @brief 发送心跳包
     * @return 0成功，-1失败
     */
    int send_heartbeat();

    /**
     * @brief 获取当前轮速数据
     * @return 轮速数据指针
     */
    const uart6_wheel_speed_t* get_wheel_speed_data() const;

    /**
     * @brief 获取当前步进电机位置数据
     * @return 步进电机位置数据指针
     */
    const uart6_stepper_pos_t* get_stepper_pos_data() const;

    /**
     * @brief 获取当前组合数据
     * @return 组合数据指针
     */
    const uart6_combined_data_t* get_combined_data() const;

    /**
     * @brief 获取统计信息
     * @return 统计信息指针
     */
    const uart6_statistics_t* get_statistics() const;

    /**
     * @brief 重置统计信息
     */
    void reset_statistics();

    /**
     * @brief 设置轮速数据回调函数
     * @param callback 回调函数指针
     */
    void set_wheel_speed_callback(void (*callback)(const uart6_wheel_speed_t* data));

    /**
     * @brief 设置步进电机位置回调函数
     * @param callback 回调函数指针
     */
    void set_stepper_pos_callback(void (*callback)(const uart6_stepper_pos_t* data));

    /**
     * @brief 设置组合数据回调函数
     * @param callback 回调函数指针
     */
    void set_combined_data_callback(void (*callback)(const uart6_combined_data_t* data));

    /**
     * @brief 设置错误回调函数
     * @param callback 回调函数指针
     */
    void set_error_callback(void (*callback)(uart6_error_t error, const char* message));

    /**
     * @brief 检查数据是否有效
     * @param max_age_ms 最大数据年龄（毫秒）
     * @return true有效，false无效
     */
    bool is_data_valid(uint32_t max_age_ms = 5000) const;

    /**
     * @brief 打印调试信息
     */
    void print_debug_info() const;
};

#endif   // __cplusplus
