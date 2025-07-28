# PD航向控制系统使用说明

## 概述

本项目已成功实现基于IMU偏航角的PD控制系统，用于两轮差速驱动机器人的航向控制和距离控制。该系统集成了现有的闭环速度控制，提供了完整的运动控制解决方案。

## 系统特点

### 1. PD航向控制
- 使用IMU偏航角反馈进行实时航向校正
- PD控制器确保机器人能够保持直线行驶或转向到指定角度
- 支持动态航向调整，精度可达2度

### 2. 距离控制
- 基于现有GMR编码器系统实现精确距离测量
- 支持前进/后退指定距离，精度可达5cm
- 自动停止功能，到达目标距离后自动停车

### 3. 集成控制模式
- **手动模式**: 直接RPM控制
- **航向保持模式**: 前进时保持指定航向
- **距离控制模式**: 行驶指定距离后停止
- **距离+航向模式**: 同时控制距离和航向
- **转向模式**: 原地转向到指定角度

## API接口说明

### 基础控制方法

```cpp
// 启用/禁用PD航向控制
motor.enableHeadingControl(true);
motor.setHeadingPDParams(0.5f, 0.1f);  // 设置Kp=0.5, Kd=0.1

// 启用/禁用距离控制
motor.enableDistanceControl(true);
motor.setTargetDistance(1.0f);  // 设置目标距离1米

// 设置目标航向
motor.setTargetHeading(90.0f);  // 设置目标航向90度
```

### 高级运动控制方法

```cpp
// 1. 带航向的前进控制
motor.moveForwardWithHeading(30.0f, 0.0f);  // 30RPM前进，保持0度航向

// 2. 距离控制
motor.moveDistance(40.0f, 1.5f);  // 40RPM前进1.5米后停止

// 3. 距离+航向控制
motor.moveDistanceWithHeading(30.0f, 2.0f, 45.0f);  // 30RPM前进2米，保持45度航向

// 4. 转向到指定角度
motor.turnToHeading(90.0f, 25.0f);  // 以25RPM转速转向到90度

// 5. 停止所有运动
motor.stop();
```

### 状态查询方法

```cpp
// 查询运动是否完成
bool completed = motor.isMovementComplete();

// 查询航向误差
float heading_error = motor.getHeadingError();

// 查询距离误差
float distance_error = motor.getDistanceError();

// 查询当前航向和距离
float current_heading = motor.getCurrentHeading();
float current_distance = motor.getCurrentDistance();
```

## 参数调整指南

### PD航向控制参数

```cpp
// 默认参数
motor.setHeadingPDParams(0.5f, 0.1f);

// 参数调整建议：
// Kp (比例增益): 0.3-0.8
//   - 过小：响应慢，航向偏差大
//   - 过大：震荡，不稳定
// Kd (微分增益): 0.05-0.2
//   - 过小：超调，震荡
//   - 过大：对噪声敏感
```

### 速度控制参数

```cpp
// 电机PID参数（已在构造函数中设置）
motor.setPIDParams(0.8f, 0.1f, 0.05f);

// 参数说明：
// Kp: 比例增益，影响响应速度
// Ki: 积分增益，消除稳态误差
// Kd: 微分增益，减少超调
```

## 测试任务说明

### 完整测试任务 (sAPP_Tasks_PDControlTest)

该任务会依次执行以下测试：

1. **航向保持测试**: 5秒钟保持0度航向前进
2. **直线行驶测试**: 5秒钟直线行驶测试
3. **转向测试**: 转向到90度
4. **距离控制测试**: 前进1米后停止
5. **距离+航向测试**: 前进0.5米并保持0度航向

### 简化演示任务 (sAPP_Tasks_SimplePDDemo)

循环演示基本功能：
- 直线前进保持航向
- 转向到指定角度
- 前进指定距离
- 停止

## 使用示例

### 示例1: 简单直线行驶

```cpp
// 初始化
motor.init();
motor.enableClosedLoopControl(true);
motor.setHeadingPDParams(0.5f, 0.1f);

// 直线前进，保持当前航向
motor.moveForwardWithHeading(30.0f, motor.getCurrentHeading());

// 在主循环中更新控制
while(1) {
    motor.update();
    motor.updateClosedLoopControl(0.02f);  // 20ms周期
    vTaskDelay(20);
}
```

### 示例2: 精确导航

```cpp
// 重置距离计数
motor.resetDistance();

// 前进2米，保持0度航向
motor.moveDistanceWithHeading(40.0f, 2.0f, 0.0f);

// 等待完成
while(!motor.isMovementComplete()) {
    motor.update();
    motor.updateClosedLoopControl(0.02f);
    vTaskDelay(20);
}

// 转向90度
motor.turnToHeading(90.0f);

// 等待转向完成
while(!motor.isMovementComplete()) {
    motor.update();
    motor.updateClosedLoopControl(0.02f);
    vTaskDelay(20);
}
```

## 注意事项

1. **IMU校准**: 确保IMU已正确校准，特别是陀螺仪零偏
2. **编码器精度**: 编码器距离测量依赖于轮径参数，需要根据实际轮径调整
3. **控制周期**: 建议使用20ms(50Hz)的控制周期以获得最佳性能
4. **参数调试**: 根据机器人的机械特性调整PD参数
5. **安全考虑**: 在测试时确保有足够的空间，避免碰撞

## 故障排除

### 常见问题

1. **航向控制不稳定**
   - 检查IMU数据是否正常
   - 降低Kp参数
   - 增加Kd参数

2. **距离控制不准确**
   - 检查编码器是否正常工作
   - 验证轮径参数设置
   - 检查编码器脉冲数设置

3. **转向不到位**
   - 检查航向控制参数
   - 验证IMU偏航角读数
   - 调整转向速度

4. **系统震荡**
   - 降低所有增益参数
   - 检查机械间隙
   - 增加阻尼（Kd参数）

## 扩展功能

该PD控制系统为后续功能扩展提供了良好的基础：

- 路径跟踪控制
- 障碍物避让
- 编队控制
- 自主导航
- SLAM集成

通过组合使用航向控制和距离控制，可以实现复杂的运动轨迹和导航任务。
