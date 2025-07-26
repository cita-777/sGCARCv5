# ZDT步进电机驱动 - STM32F405移植版

## 概述

本驱动是ZDT步进电机库的STM32F405移植版本，原始版本为ESP32平台设计。已完全重写以适配STM32 HAL库和sGCARC项目架构。

## 硬件配置

### UART配置
- **UART端口**: UART3
- **引脚配置**:
  - TX: PB10 (MOTOR_TX_Pin)
  - RX: PB11 (MOTOR_RX_Pin)
- **波特率**: 115200 (可配置)
- **数据位**: 8位
- **停止位**: 1位
- **校验位**: 无

### 引脚配置说明
- 使用UART3的PB10(TX)和PB11(RX)引脚
- 原IMU功能已禁用，UART3现在专用于步进电机
- 通过注释掉IMU相关代码来避免冲突

## 软件架构

### 文件结构
```
sDRV/
├── sDRV_zdt_motor.h      # 头文件
├── sDRV_zdt_motor.cpp    # 实现文件
└── sDRV_zdt_motor_README.md  # 本文档
```

### 依赖关系
- `sBSP_UART.h/c` - BSP层UART API
- `sDBG_Debug.h` - 调试输出
- `stm32f4xx_hal.h` - STM32 HAL库

## API接口

### 构造函数
```cpp
sDRV_ZDTMotor(uint8_t address = 1, uint32_t baud_rate = 115200, uint8_t data_check_byte = 0x6B);
```

### 状态查询函数
```cpp
uint16_t get_encoder_value();        // 读取编码器值
uint32_t get_pulse_value();          // 读取输入脉冲数
uint32_t get_motor_position();       // 读取电机实时位置
uint16_t get_motor_position_err();   // 读取电机位置误差
uint8_t get_en_status();             // 读取使能状态
uint8_t get_protect_watermark();     // 读取堵转标志
uint8_t get_o_status();              // 读取单圈上电自动回零状态标志
```

### 控制函数
```cpp
uint8_t ctrl_en(uint8_t enabled);                    // 控制使能状态
uint8_t ctrl_speed_mode(uint8_t direction, uint8_t speed, uint8_t acceleration);  // 速度模式控制
uint8_t ctrl_pos_mode(uint8_t direction, uint8_t speed, uint8_t acceleration, 
                     uint8_t pulse1, uint8_t pulse2, uint8_t pulse3);  // 位置模式控制
uint8_t clear_speed_mode();          // 清除速度模式参数
uint8_t save_speed_mode();           // 存储速度模式参数
uint8_t cancel_motor_protect();      // 解除堵转保护
```

### 配置函数
```cpp
uint8_t edit_m_step(uint8_t m_step);     // 修改细分步数
uint8_t edit_uart_addr(uint8_t addr);    // 修改串口通讯地址
```

## 使用示例

### 基本使用
```cpp
#include "sDRV_zdt_motor.h"

// 创建步进电机实例
sDRV_ZDTMotor motor(1, 115200, 0x6B);

// 使能电机
motor.ctrl_en(1);

// 速度模式控制 - 正转，速度50，加速度10
motor.ctrl_speed_mode(1, 50, 10);

// 查询电机状态
uint8_t status = motor.get_en_status();
uint32_t position = motor.get_motor_position();
```

### 应用层集成
步进电机测试任务已集成到应用层任务系统中：
- 任务名称: `sAPP_Tasks_StepperMotorTest`
- 任务优先级: 2
- 栈大小: 4096字节
- 测试周期: 5秒

## 注意事项

### 当前限制
1. **接收功能简化**: 当前版本使用模拟接收数据，实际项目中需要实现完整的UART接收机制
2. **同步通信**: 当前使用阻塞式发送，可考虑改为异步通信以提高性能
3. **错误处理**: 建议添加更完善的错误处理和重试机制

### 扩展建议
1. 实现基于中断或DMA的异步UART通信
2. 添加命令队列机制支持批量操作
3. 实现电机状态监控和故障诊断
4. 添加配置参数持久化存储

## 测试验证

### 编译验证
- ✅ 头文件包含正确
- ✅ 函数声明匹配
- ✅ 无编译错误或警告

### 硬件验证
- ✅ UART3引脚配置正确 (PB10/PB11)
- ✅ 中断处理函数已配置
- ✅ BSP层API集成完成
- ✅ IMU功能已正确禁用

### 功能验证
- ✅ 任务创建和调度
- ✅ 基本命令发送
- ⚠️ 实际硬件通信需要连接真实步进电机进行测试

## 版本历史

- v0.2 - STM32F405移植版
  - 完全重写以适配STM32平台
  - 集成到sGCARC项目架构
  - 使用UART3 (PB10/PB11)，禁用IMU功能
  - 添加BSP层API支持

- v0.1beta - 原始ESP32版本
  - 支持ESP32平台
  - 使用ESP-IDF框架
