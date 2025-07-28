# UART6主机通信驱动程序

## 概述

本驱动程序为STM32F405微控制器的UART6端口提供了与主机计算机进行通信的功能。采用基于文本的键值对协议，易于阅读和调试，支持轮速控制和步进电机位置命令的接收与解析。

## 特性

- **文本协议**：使用类似字典的键值对格式，如 `{cmd:wheel,lw:1.5,rw:2.0}`
- **易于扩展**：模块化设计，可轻松添加新的数据字段和命令类型
- **实时解析**：支持实时数据包解析和回调通知
- **错误处理**：完善的错误检测和处理机制
- **统计信息**：提供详细的通信统计信息
- **回调机制**：支持多种数据类型的回调函数

## 硬件配置

### UART6配置
- **UART端口**: UART6
- **引脚配置**: 根据STM32F405引脚定义
- **波特率**: 115200（可配置）
- **数据位**: 8位
- **停止位**: 1位
- **校验位**: 无

## 软件架构

### 文件结构
```
sDRV/
├── sDRV_UART6_HostComm.h          # 头文件
├── sDRV_UART6_HostComm.cpp        # 实现文件
├── sDRV_UART6_HostComm_test.h     # 测试头文件
├── sDRV_UART6_HostComm_test.cpp   # 测试实现文件
└── sDRV_UART6_HostComm_README.md  # 本文档
```

### 依赖关系
- `sBSP_UART.h/c` - BSP层UART API
- `sDBG_Debug.h` - 调试输出
- `stm32f4xx_hal.h` - STM32 HAL库

## 协议格式

### 基本格式
```
{key1:value1,key2:value2,key3:value3}
```

### 支持的命令类型

#### 1. 轮速控制命令
```
{cmd:wheel,lw:1.5,rw:2.0}
```
- `cmd`: 命令类型，固定为"wheel"
- `lw`: 左轮速度（浮点数）
- `rw`: 右轮速度（浮点数）

#### 2. 步进电机位置命令
```
{cmd:stepper,s1:1000,s2:-500}
```
- `cmd`: 命令类型，固定为"stepper"
- `s1`: 步进电机1位置（整数）
- `s2`: 步进电机2位置（整数）

#### 3. 组合数据命令
```
{cmd:combined,lw:1.2,rw:1.8,s1:800,s2:200}
```
- `cmd`: 命令类型，固定为"combined"
- `lw`,`rw`: 左右轮速度
- `s1`,`s2`: 步进电机1,2位置

#### 4. 心跳包
```
{cmd:heartbeat}
```
- `cmd`: 命令类型，固定为"heartbeat"

#### 5. 状态请求
```
{cmd:status}
```
- `cmd`: 命令类型，固定为"status"

#### 6. 复位命令
```
{cmd:reset}
```
- `cmd`: 命令类型，固定为"reset"

## API接口

### 构造函数
```cpp
sDRV_UART6_HostComm(uint32_t baud_rate = 115200, uint32_t timeout_ms = 1000);
```

### 初始化和控制
```cpp
int init();                    // 初始化UART6
int start_receive();           // 开始接收数据
void stop_receive();           // 停止接收数据
```

### 数据获取
```cpp
const uart6_wheel_speed_t* get_wheel_speed_data() const;
const uart6_stepper_pos_t* get_stepper_pos_data() const;
const uart6_combined_data_t* get_combined_data() const;
const uart6_statistics_t* get_statistics() const;
```

### 回调函数设置
```cpp
void set_wheel_speed_callback(void (*callback)(const uart6_wheel_speed_t* data));
void set_stepper_pos_callback(void (*callback)(const uart6_stepper_pos_t* data));
void set_combined_data_callback(void (*callback)(const uart6_combined_data_t* data));
void set_error_callback(void (*callback)(uart6_error_t error, const char* message));
```

### 发送功能
```cpp
int send_response(uart6_cmd_type_t cmd, const uint8_t* payload, uint8_t length);
int send_heartbeat();
```

### 实用功能
```cpp
bool is_data_valid(uint32_t max_age_ms = 5000) const;
void reset_statistics();
void print_debug_info() const;
```

## 使用示例

### 基本使用
```cpp
#include "sDRV_UART6_HostComm.h"

// 创建驱动实例
sDRV_UART6_HostComm uart6_comm(115200, 2000);

// 回调函数
void wheel_callback(const uart6_wheel_speed_t* data) {
    printf("轮速: 左=%.2f, 右=%.2f\n", 
           data->left_wheel_speed, data->right_wheel_speed);
}

void stepper_callback(const uart6_stepper_pos_t* data) {
    printf("步进电机: 1=%ld, 2=%ld\n", 
           data->stepper1_position, data->stepper2_position);
}

// 初始化
int main() {
    // 初始化驱动
    if (uart6_comm.init() != 0) {
        printf("初始化失败\n");
        return -1;
    }
    
    // 设置回调函数
    uart6_comm.set_wheel_speed_callback(wheel_callback);
    uart6_comm.set_stepper_pos_callback(stepper_callback);
    
    // 开始接收
    uart6_comm.start_receive();
    
    // 主循环
    while (1) {
        // 检查数据有效性
        if (uart6_comm.is_data_valid()) {
            // 处理有效数据
        }
        
        delay(100);
    }
}
```

### FreeRTOS任务集成
```cpp
extern "C" void uart6_comm_task(void* param) {
    sDRV_UART6_HostComm uart6_comm;
    
    // 初始化
    uart6_comm.init();
    uart6_comm.start_receive();
    
    for (;;) {
        // 定期检查统计信息
        const uart6_statistics_t* stats = uart6_comm.get_statistics();
        if (stats->total_packets > 0) {
            printf("收到 %lu 个数据包\n", stats->total_packets);
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
```

## 测试

### 运行测试
1. 包含测试头文件：
```cpp
#include "sDRV_UART6_HostComm_test.h"
```

2. 创建测试任务：
```cpp
xTaskCreate(sAPP_Tasks_UART6_HostCommTest, "UART6Test", 4096, NULL, 2, NULL);
```

3. 通过串口发送测试数据包进行验证

### 测试数据包
```
{cmd:wheel,lw:1.5,rw:2.0}
{cmd:stepper,s1:1000,s2:-500}
{cmd:combined,lw:1.2,rw:1.8,s1:800,s2:200}
{cmd:heartbeat}
```

## 扩展指南

### 添加新的数据字段
1. 在头文件中定义新的键名常量
2. 在数据结构中添加新字段
3. 在解析函数中添加新字段的处理逻辑
4. 更新回调函数接口

### 添加新的命令类型
1. 在枚举中添加新的命令类型
2. 定义命令字符串常量
3. 在命令解析函数中添加处理逻辑
4. 实现相应的数据处理函数

## 故障排除

### 常见问题
1. **初始化失败**：检查UART6硬件配置和引脚定义
2. **收不到数据**：检查UART连接和波特率设置
3. **解析失败**：检查数据包格式是否符合协议规范
4. **回调不执行**：确认回调函数已正确设置且不为空

### 调试方法
1. 使用 `print_debug_info()` 查看详细状态
2. 检查统计信息中的错误计数
3. 启用调试输出查看接收到的原始数据
4. 使用示波器检查UART信号质量

## 版本历史

- **v1.0** (2024-12-19)
  - 初始版本
  - 支持基本的轮速和步进电机数据解析
  - 实现文本协议和回调机制
  - 添加完整的测试套件

## 许可证

本驱动程序遵循项目的整体许可证协议。
