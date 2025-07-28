#include "sAPP_Tasks.hpp"

#include "main.h"
#include "sAPP_AHRS.hpp"
#include "sAPP_Car.hpp"
#include "sAPP_Motor.hpp"
// 删除了超声波和语音相关的头文件包含
#include "sBSP_TIM.h"
#include "sDBG_Debug.h"
#include "sDRV_GrayScale.h"
#include "sDRV_PwrLight.h"
#include "sDRV_UART6_HostComm.h"
#include "sDRV_zdt_motor.h"
#include "sDWTLib.hpp"
#include <math.h>
#include <stdio.h>
#include <string.h>


// 方向播报任务相关变量已删除




void sAPP_Tasks_OLEDHdr(void* param)
{
    for (;;)
    {
        menu.update();

        oled.handler();
        oled.setAll(0);

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


void sAPP_Tasks_Devices(void* param)
{
    for (;;)
    {
        // 读取PS2手柄数据
        // sDRV_PS2_Handler();
        // 处理按键
        sGBD_Handler();
        // 处理二值化设备
        BinOutDrv.update();



        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}


// 超声波测距相关功能已删除 - 差速两轮小车不需要

// 步进电机测试任务
void sAPP_Tasks_StepperMotorTest(void* param)
{
    dbg_printf("[INFO] 双电机位置模式测试任务启动\n");

    // 确保PwrLight供电
    sDRV_PL_SetBrightness(100.0f);
    dbg_printf("[INFO] PwrLight设置为100%%，步进电机供电正常\n");

    // 创建两个步进电机实例
    // 1号电机：云台底座（水平旋转）
    static sDRV_ZDTMotor motor_base(1, 115200, 0x6B);
    // 2号电机：Y轴控制（垂直运动）
    static sDRV_ZDTMotor motor_y_axis(2, 115200, 0x6B);
    dbg_printf("[INFO] 双电机实例创建完成 - 1号:云台底座, 2号:Y轴\n");

    // 等待一段时间让系统稳定
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    for (;;)
    {
        dbg_printf("[INFO] =====双电机位置模式运动测试=====\n");

        // 1. 使能两个电机
        dbg_printf("[INFO] 使能1号电机(云台底座)...\n");
        motor_base.ctrl_en(1);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        if (motor_base.is_recv_complete())
        {
            dbg_printf("[INFO] 1号电机使能成功: 0x%02X\n", motor_base.get_recv_result());
        }
        else
        {
            dbg_printf("[WARN] 1号电机使能无响应\n");
        }

        dbg_printf("[INFO] 使能2号电机(Y轴)...\n");
        motor_y_axis.ctrl_en(1);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        if (motor_y_axis.is_recv_complete())
        {
            dbg_printf("[INFO] 2号电机使能成功: 0x%02X\n", motor_y_axis.get_recv_result());
        }
        else
        {
            dbg_printf("[WARN] 2号电机使能无响应\n");
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);

        // 2. 查询初始位置
        dbg_printf("[INFO] 查询1号电机初始位置...\n");
        motor_base.get_motor_position();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (motor_base.is_recv_complete())
        {
            dbg_printf("[INFO] 1号电机初始位置: %u\n", motor_base.get_recv_result());
        }

        dbg_printf("[INFO] 查询2号电机初始位置...\n");
        motor_y_axis.get_motor_position();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (motor_y_axis.is_recv_complete())
        {
            dbg_printf("[INFO] 2号电机初始位置: %u\n", motor_y_axis.get_recv_result());
        }

        // 3. 位置模式运动测试 - 小距离移动
        dbg_printf("[INFO] 开始位置模式运动测试...\n");

        // 1号电机(云台底座)：顺时针旋转小角度
        // 参数：方向0=CW, 速度30RPM, 加速度3, 脉冲数1000(小角度), 相对模式0
        dbg_printf("[INFO] 1号电机顺时针旋转小角度...\n");
        motor_base.ctrl_pos_mode(0, 30, 3, 1000, 0);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        if (motor_base.is_recv_complete())
        {
            dbg_printf("[INFO] 1号电机位置命令响应: 0x%02X\n", motor_base.get_recv_result());
        }
        else
        {
            dbg_printf("[WARN] 1号电机位置命令无响应\n");
        }

        // 2号电机(Y轴)：向上移动小距离
        // 参数：方向1=CCW, 速度25RPM, 加速度3, 脉冲数800(小距离), 相对模式0
        dbg_printf("[INFO] 2号电机向上移动小距离...\n");
        motor_y_axis.ctrl_pos_mode(1, 25, 3, 800, 0);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        if (motor_y_axis.is_recv_complete())
        {
            dbg_printf("[INFO] 2号电机位置命令响应: 0x%02X\n", motor_y_axis.get_recv_result());
        }
        else
        {
            dbg_printf("[WARN] 2号电机位置命令无响应\n");
        }

        // 等待运动完成
        dbg_printf("[INFO] 等待运动完成...\n");
        vTaskDelay(4000 / portTICK_PERIOD_MS);

        // 4. 查询运动后位置
        dbg_printf("[INFO] 查询1号电机运动后位置...\n");
        motor_base.get_motor_position();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (motor_base.is_recv_complete())
        {
            dbg_printf("[INFO] 1号电机运动后位置: %u\n", motor_base.get_recv_result());
        }

        dbg_printf("[INFO] 查询2号电机运动后位置...\n");
        motor_y_axis.get_motor_position();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (motor_y_axis.is_recv_complete())
        {
            dbg_printf("[INFO] 2号电机运动后位置: %u\n", motor_y_axis.get_recv_result());
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // 5. 反向运动回到初始位置
        dbg_printf("[INFO] 反向运动回到初始位置...\n");

        // 1号电机逆时针回转
        dbg_printf("[INFO] 1号电机逆时针回转...\n");
        motor_base.ctrl_pos_mode(1, 30, 3, 1000, 0);
        vTaskDelay(300 / portTICK_PERIOD_MS);

        // 2号电机向下回移
        dbg_printf("[INFO] 2号电机向下回移...\n");
        motor_y_axis.ctrl_pos_mode(0, 25, 3, 800, 0);
        vTaskDelay(300 / portTICK_PERIOD_MS);

        // 等待回移完成
        dbg_printf("[INFO] 等待回移完成...\n");
        vTaskDelay(4000 / portTICK_PERIOD_MS);

        // 6. 禁用电机
        dbg_printf("[INFO] 禁用电机...\n");
        motor_base.ctrl_en(0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        motor_y_axis.ctrl_en(0);
        vTaskDelay(200 / portTICK_PERIOD_MS);

        // 每15秒执行一次测试
        dbg_printf("[INFO] 双电机位置测试完成，等待下一轮...\n");
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
}



void sAPP_Tasks_TaskMang(void* param)
{

    for (;;)
    {
        sAPP_Tasks_PrintTaskMang();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void sAPP_Tasks_LoopTask(void* param)
{
    for (;;)
    {
        loop();
    }
}



void sAPP_Tasks_FormatFeRAM(void* param)
{
    menu.setLock("FeRAM", "FeRAM is \nformatting...");
    sBSP_UART_Debug_Printf("FeRAM开始格式化!\n");
    if (sDRV_MB85RCxx_Format(0) == 0)
    {
        sBSP_UART_Debug_Printf("FeRAM格式化完成!\n");
    }
    else
    {
        sBSP_UART_Debug_Printf("FeRAM格式化失败!\n");
    }
    menu.setUnlock();
    vTaskDelete(NULL);
}

static void calibrateIMU(void* param)
{
    menu.setLock("IMU", "IMU is \ncalibrating...");
    sAPP_ParamSave_CaliIMU();




    menu.setUnlock();
    vTaskDelete(NULL);
}


static void imu_calib_acc_bias(void* param)
{
    menu.setLock("IMU Calib", "Accel is \ncalibrating...");
    log_info("在1s后开始校准加速度计静态零偏...");
    vTaskDelay(1000);

    // 校准加速度计静态零偏
    AHRS::IMU_StaticBias imu_sbias;
    // 静态零偏采样点数
    const uint16_t sample_points = 2000;
    ahrs.calcBias(sample_points, imu_sbias);
    float temp = ahrs.raw_data.imu_temp;

    log_info("测量点数:%u,加速度计静态零偏:%.3f,%.3f,%.3f",
             sample_points,
             imu_sbias.acc_x,
             imu_sbias.acc_y,
             imu_sbias.acc_z);

    // 静态零偏阈值,超过说明没有把系统放平
    const float bias_limit = 0.3f;
    if (fabs(imu_sbias.acc_x) > bias_limit || fabs(imu_sbias.acc_y) > bias_limit || fabs(imu_sbias.acc_z) > bias_limit)
    {
        log_error("加速度计静态零偏数据超过阈值,请将主板放平后重试! 本次校准无效");
        menu.setUnlock();
        // 提示用户
        menu.createTipsBox("Acc Calib FAIL", "Plase take the\nsystem on\nflat surface!");
        goto END;
    }

    // 保存静态零偏数据到FeRAM
    if (sAPP_ParamSave_SaveAccSBias(imu_sbias.acc_x, imu_sbias.acc_y, imu_sbias.acc_z, temp) != 0)
    {
        log_error("加速度计静态零偏数据保存失败!");
        // 提示用户
        menu.createTipsBox("Acc Calib FAIL", "Static bias\nsave fail!");
        goto END;
    }

    // 保存静态零偏数据到AHRS
    ahrs.updateAccSBias(imu_sbias.acc_x, imu_sbias.acc_y, imu_sbias.acc_z);

    log_info("加速度计静态零偏校准完成,保存成功 OK");
    menu.createTipsBox("Acc Calib OK", "Static bias\nsave success!");


END:
    menu.setUnlock();
    vTaskDelete(NULL);
}

static void imu_calib_gyr_bias(void* param)
{
    menu.setLock("IMU Calib", "Gyro is \ncalibrating...");
    log_info("在1s后开始校准陀螺仪静态零偏...");
    vTaskDelay(1000);

    // 校准陀螺仪静态零偏
    AHRS::IMU_StaticBias imu_sbias;
    // 静态零偏采样点数
    const uint16_t sample_points = 2000;
    float          temp          = ahrs.raw_data.imu_temp;
    ahrs.calcBias(sample_points, imu_sbias);

    log_info(
        "测量点数:%u,陀螺仪静态零偏:%.3f,%.3f,%.3f", sample_points, imu_sbias.gyr_x, imu_sbias.gyr_y, imu_sbias.gyr_z);

    // 静态零偏阈值,超过说明系统没有在静止状态或者陀螺仪有问题
    const float bias_limit = ahrs.getIMUType() == AHRS::IMUType::ICM45686 ? 0.5f : 0.5f;
    if (fabs(imu_sbias.gyr_x) > bias_limit || fabs(imu_sbias.gyr_y) > bias_limit || fabs(imu_sbias.gyr_z) > bias_limit)
    {
        log_error("陀螺仪静态零偏数据超过阈值,可能是系统未处于静止状态或者陀螺仪性能不佳 本次校准无效");
        menu.createTipsBox("Gyro Calib FAIL", "Plase keep the\nsystem in a\nstatic state!");

        goto END;
    }

    // 保存静态零偏数据到FeRAM
    if (sAPP_ParamSave_SaveGyrSBias(imu_sbias.gyr_x, imu_sbias.gyr_y, imu_sbias.gyr_z, temp) != 0)
    {
        log_error("陀螺仪静态零偏数据保存失败!");
        menu.createTipsBox("Gyro Calib FAIL", "Static bias\nsave fail!");
        goto END;
    }

    // 保存静态零偏数据到AHRS
    ahrs.updateGyrSBias(imu_sbias.gyr_x, imu_sbias.gyr_y, imu_sbias.gyr_z);

    log_info("陀螺仪静态零偏校准完成,保存成功 OK");
    menu.createTipsBox("Gyro Calib OK", "Static bias\nsave success!");

END:
    menu.setUnlock();
    vTaskDelete(NULL);
}


void sAPP_Tasks_ReadIMUCaliVal()
{
    AHRS::IMU_StaticBias imu_sbias;
    float                acc_bias_temp     = 0.0f;
    float                gyr_bias_temp     = 0.0f;
    bool                 is_acc_bias_valid = false;
    bool                 is_gyr_bias_valid = false;
    if (sAPP_ParamSave_ReadAccSBias(&imu_sbias.acc_x, &imu_sbias.acc_y, &imu_sbias.acc_z, &acc_bias_temp) != 0)
    {
        log_error("加速度计静态零偏数据无效!");
        is_acc_bias_valid = false;
    }
    else
    {
        // 检查校准时的温度和当前温度差
        ahrs.getIMUData();
        float diff_temp = fabs(acc_bias_temp - ahrs.raw_data.imu_temp);
        if (diff_temp > 3.0f)
        {
            log_warn("加速度计静态零偏数据温度差过大,建议重新校准,偏差:%.1f摄氏度", diff_temp);
            is_acc_bias_valid = false;
        }
        else
        {
            is_acc_bias_valid = true;
        }
    }

    if (sAPP_ParamSave_ReadGyrSBias(&imu_sbias.gyr_x, &imu_sbias.gyr_y, &imu_sbias.gyr_z, &gyr_bias_temp) != 0)
    {
        log_error("陀螺仪静态零偏数据无效!");
        is_gyr_bias_valid = false;
    }
    else
    {
        // 检查校准时的温度和当前温度差
        ahrs.getIMUData();
        float diff_temp = fabs(gyr_bias_temp - ahrs.raw_data.imu_temp);
        if (diff_temp > 3.0f)
        {
            log_warn("陀螺仪静态零偏数据温度差过大,建议重新校准,偏差:%.1f摄氏度", diff_temp);
            is_gyr_bias_valid = false;
        }
        else
        {
            is_gyr_bias_valid = true;
        }
        is_gyr_bias_valid = true;
    }

    if (is_acc_bias_valid == true)
    {
        ahrs.updateAccSBias(imu_sbias.acc_x, imu_sbias.acc_y, imu_sbias.acc_z);
        log_info("加速度计静态零偏数据已读取:%.4f,%.4f,%.4f", imu_sbias.acc_x, imu_sbias.acc_y, imu_sbias.acc_z);
        ahrs.setIMUState(AHRS::IMUState::OK);
    }
    else
    {
        ahrs.setIMUState(AHRS::IMUState::NEED_CALIB);
    }

    if (is_gyr_bias_valid == true)
    {
        ahrs.updateGyrSBias(imu_sbias.gyr_x, imu_sbias.gyr_y, imu_sbias.gyr_z);
        log_info("陀螺仪静态零偏数据已读取:%.4f,%.4f,%.4f", imu_sbias.gyr_x, imu_sbias.gyr_y, imu_sbias.gyr_z);
        ahrs.setIMUState(AHRS::IMUState::OK);
    }
    else
    {
        ahrs.setIMUState(AHRS::IMUState::NEED_CALIB);
    }
}

void sAPP_Tasks_ProtectTask(void* param)
{
    bool is_low_bat   = false;
    bool is_first_tip = true;

    vTaskDelay(5000);
    for (;;)
    {
        if (xSemaphoreTake(car.mutex, 200) == pdTRUE)
        {
            // 电池电压过低保护
            if (car.batt_volt < 10.5f)
            {
                is_low_bat = true;
                // sBSP_UART_Debug_Printf("[ERR ]电池电压过低:%.2fv\n",car.batt_volt);
            }
            // 回差500mV,防止重复触发
            else if (car.batt_volt > 11.0f)
            {
                is_low_bat   = false;
                is_first_tip = true;
            }
            xSemaphoreGive(car.mutex);
        }

        if (is_low_bat && is_first_tip)
        {
            is_first_tip = false;
            menu.createTipsBox("LOW BATT WARN", "Battery voltage\nis too low!");
            // 语音播报功能已删除，只保留蜂鸣器警告
            BinOutDrv.startPulse(BOD_BUZZER_ID);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            BinOutDrv.startPulse(BOD_BUZZER_ID);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            BinOutDrv.startPulse(BOD_BUZZER_ID);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

// 差速两轮小车电机控制任务 - 闭环速度控制
void sAPP_Tasks_MotorControlTask(void* param)
{
    sBSP_UART_Debug_Printf("[INFO] 电机闭环速度控制任务启动\n");

    // 初始化电机
    motor.init();

    // 启用闭环控制
    motor.enableClosedLoopControl(true);

    // 设置初始PID参数（可以根据实际调试结果调整）
    motor.setPIDParams(0.6f, 1.2f, 0.0f);

    sBSP_UART_Debug_Printf("[INFO] 电机闭环控制已启用，PID参数: Kp=0.8, Ki=0.1, Kd=0.05\n");

    vTaskDelay(100 / portTICK_PERIOD_MS);   // 等待初始化完成

    // 控制周期参数
    const float CONTROL_DT    = 0.02f;   // 20ms控制周期，50Hz
    TickType_t  xLastWakeTime = xTaskGetTickCount();

    // 测试变量
    uint32_t test_counter    = 0;
    float    test_target_rpm = 0.0f;

    for (;;)
    {
        // 更新电机编码器数据
        motor.update();

        // 简单的测试逻辑：每5秒改变一次目标转速
        test_counter++;
        if (test_counter >= 50)
        {   // 250 * 20ms = 5秒
            test_counter = 0;

            // 循环测试不同的转速：0 -> 30 -> -30 -> 60 -> -60 -> 0
            static int test_phase = 0;
            switch (test_phase)
            {
            case 0: test_target_rpm = 30.0f; break;
            case 1: test_target_rpm = -30.0f; break;
            case 2: test_target_rpm = 120.0f; break;
            case 3: test_target_rpm = -120.0f; break;
            case 4: test_target_rpm = 0.0f; break;
            }
            test_phase = (test_phase + 1) % 5;

            // 设置目标转速
            motor.setTargetRPM(test_target_rpm, test_target_rpm);
            sBSP_UART_Debug_Printf("[INFO] 设置目标转速: %.1f RPM\n", test_target_rpm);
        }

        // 执行闭环控制更新
        motor.updateClosedLoopControl(CONTROL_DT);

        // 每秒打印一次状态信息
        static uint32_t print_counter = 0;
        print_counter++;
        if (print_counter >= 5)
        {   // 50 * 20ms = 1秒
            print_counter = 0;

            float left_rpm     = motor.getLRPM();
            float right_rpm    = motor.getRRPM();
            float left_target  = motor.getLeftTargetRPM();
            float right_target = motor.getRightTargetRPM();

            sBSP_UART_Debug_Printf("%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
                                   left_target,
                                   right_target,
                                   left_rpm,
                                   right_rpm,
                                   left_target - left_rpm,
                                   right_target - right_rpm);
        }

        // 精确的20ms周期控制
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

// 灰度传感器测试任务
void sAPP_Tasks_GrayScaleTest(void* param)
{
    dbg_printf("[INFO] 灰度传感器测试任务启动\n");

    // 初始化灰度传感器
    if (sDRV_GrayScale_Init() != 0)
    {
        dbg_printf("[ERROR] 灰度传感器初始化失败\n");
        vTaskDelete(NULL);
        return;
    }

    dbg_printf("[INFO] 灰度传感器初始化成功\n");

    // 等待系统稳定
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // 测试变量
    uint8_t digital_data;
    uint8_t analog_data[8]     = {0};
    uint8_t normalized_data[8] = {0};
    char    output_buffer[256] = {0};

    for (;;)
    {
        dbg_printf("[INFO] =====灰度传感器测试=====\n");

        // 测试Ping功能
        if (sDRV_GrayScale_Ping() == 0)
        {
            dbg_printf("[INFO] 传感器Ping成功\n");
        }
        else
        {
            dbg_printf("[WARN] 传感器Ping失败，请检查连接\n");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        // 获取数字模式数据
        digital_data = sDRV_GrayScale_GetDigital();
        snprintf(output_buffer,
                 sizeof(output_buffer),
                 "[INFO] 数字模式: %d-%d-%d-%d-%d-%d-%d-%d\n",
                 (digital_data >> 0) & 0x01,
                 (digital_data >> 1) & 0x01,
                 (digital_data >> 2) & 0x01,
                 (digital_data >> 3) & 0x01,
                 (digital_data >> 4) & 0x01,
                 (digital_data >> 5) & 0x01,
                 (digital_data >> 6) & 0x01,
                 (digital_data >> 7) & 0x01);
        dbg_printf("%s", output_buffer);

        // 获取模拟模式数据
        if (sDRV_GrayScale_GetAnalog(analog_data, 8) == 0)
        {
            snprintf(output_buffer,
                     sizeof(output_buffer),
                     "[INFO] 模拟模式: %d-%d-%d-%d-%d-%d-%d-%d\n",
                     analog_data[0],
                     analog_data[1],
                     analog_data[2],
                     analog_data[3],
                     analog_data[4],
                     analog_data[5],
                     analog_data[6],
                     analog_data[7]);
            dbg_printf("%s", output_buffer);
        }
        else
        {
            dbg_printf("[WARN] 模拟数据读取失败\n");
        }

        // 设置归一化并获取归一化数据
        if (sDRV_GrayScale_SetNormalize(0xFF) == 0)
        {                                          // 所有通道归一化
            vTaskDelay(10 / portTICK_PERIOD_MS);   // 等待传感器处理

            if (sDRV_GrayScale_GetAnalog(normalized_data, 8) == 0)
            {
                snprintf(output_buffer,
                         sizeof(output_buffer),
                         "[INFO] 归一化数据: %d-%d-%d-%d-%d-%d-%d-%d\n",
                         normalized_data[0],
                         normalized_data[1],
                         normalized_data[2],
                         normalized_data[3],
                         normalized_data[4],
                         normalized_data[5],
                         normalized_data[6],
                         normalized_data[7]);
                dbg_printf("%s", output_buffer);
            }
            else
            {
                dbg_printf("[WARN] 归一化数据读取失败\n");
            }

            // 关闭归一化
            sDRV_GrayScale_SetNormalize(0x00);
        }
        else
        {
            dbg_printf("[WARN] 归一化设置失败\n");
        }

        // 测试单个通道读取
        dbg_printf("[INFO] 单通道测试: ");
        for (uint8_t i = 1; i <= 8; i++)
        {
            uint8_t single_val = sDRV_GrayScale_GetSingleAnalog(i);
            dbg_printf("CH%d:%d ", i, single_val);
        }
        dbg_printf("\n");

        // 获取偏移值
        uint16_t offset = sDRV_GrayScale_GetOffset();
        dbg_printf("[INFO] 偏移值: %u\n", offset);

        dbg_printf("[INFO] 测试完成，等待下一轮...\n");
        vTaskDelay(2000 / portTICK_PERIOD_MS);   // 2秒间隔
    }
}




void sAPP_Tasks_CreateAll()
{
    // 差速两轮小车核心任务

    // 姿态估计算法 - 提供IMU数据和姿态信息
    xTaskCreate(sAPP_AHRS_Task, "AHRS", 16384 / sizeof(int), NULL, 5, NULL);

    // OLED显示任务 - 用户界面显示
    xTaskCreate(sAPP_Tasks_OLEDHdr, "OLED", 16384 / sizeof(int), NULL, 2, NULL);

    // 设备处理任务 - 按键等基础设备
    xTaskCreate(sAPP_Tasks_Devices, "Devices", 1024 / sizeof(int), NULL, 1, NULL);

    // 小车信息更新任务 - 电池状态等
    xTaskCreate(sAPP_Car_InfoUpdateTask, "CarInfoUp", 2048 / sizeof(int), NULL, 1, NULL);

    // 保护任务 - 基础保护功能
    xTaskCreate(sAPP_Tasks_ProtectTask, "Protect", 2048 / sizeof(int), NULL, 1, NULL);

    // 主循环任务
    xTaskCreate(sAPP_Tasks_LoopTask, "Loop", 8192 / sizeof(int), NULL, 2, NULL);

    // 电机控制任务 - 差速两轮小车核心功能（闭环速度控制）
    // xTaskCreate(sAPP_Tasks_MotorControlTask, "MotorCtrl", 4096 / sizeof(int), NULL, 4, NULL);

    // 步进电机测试任务
    // xTaskCreate(sAPP_Tasks_StepperMotorTest, "StepperTest", 4096 / sizeof(int), NULL, 2, NULL);

    // 灰度传感器测试任务
    // xTaskCreate(sAPP_Tasks_GrayScaleTest, "GrayScaleTest", 4096 / sizeof(int), NULL, 2, NULL);

    // UART6主机通信测试任务
    // xTaskCreate(sAPP_Tasks_UART6_HostCommTest, "UART6Test", 4096 / sizeof(int), NULL, 2, NULL);
}


void sAPP_Tasks_StartFormatFeRAM()
{
    xTaskCreate(sAPP_Tasks_FormatFeRAM, "ForatFeRAM", 512 / sizeof(int), NULL, 1, NULL);
}

void sAPP_Tasks_StartCalibrateIMU()
{
    xTaskCreate(calibrateIMU, "calibrateIMU", 1024 / sizeof(int), NULL, 1, NULL);
}

void sAPP_Tasks_StartCalibAccBias()
{
    xTaskCreate(imu_calib_acc_bias, "calibAccBias", 1024 / sizeof(int), NULL, 2, NULL);
}

void sAPP_Tasks_StartCalibGyrBias()
{
    xTaskCreate(imu_calib_gyr_bias, "calibGyrBias", 1024 / sizeof(int), NULL, 2, NULL);
}



void sAPP_Tasks_PrintTaskMang()
{
    const char           task_state[] = {'r', 'R', 'B', 'S', 'D'};
    volatile UBaseType_t uxArraySize, x;
    unsigned portSHORT   ulTotalRunTime, ulStatsAsPercentage;
    /* 获取任务总数目 */
    uxArraySize = uxTaskGetNumberOfTasks();
    sBSP_UART_Debug_Printf("当前任务数量:%d\n", uxArraySize);
    TaskStatus_t pxTaskStatusArray[16];
    /*获取每个任务的状态信息 */
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);
    sBSP_UART_Debug_Printf("任务名        状态       ID       优先级       堆栈剩余      CPU使用率\n");
    /* 避免除零错误 */
    if (ulTotalRunTime > 0)
    {
        /* 将获得的每一个任务状态信息部分的转化为程序员容易识别的字符串格式 */
        for (x = 0; x < uxArraySize; x++)
        {
            char tmp[128];
            /* 计算任务运行时间与总运行时间的百分比。*/
            ulStatsAsPercentage = (uint16_t)(pxTaskStatusArray[x].ulRunTimeCounter) * 100 / ulTotalRunTime;

            if (ulStatsAsPercentage > 0UL)
            {
                sprintf(tmp,
                        "%-15s%-10c%-10lu%-12lu%-12d%d%%",
                        pxTaskStatusArray[x].pcTaskName,
                        task_state[pxTaskStatusArray[x].eCurrentState],
                        pxTaskStatusArray[x].xTaskNumber,
                        pxTaskStatusArray[x].uxCurrentPriority,
                        pxTaskStatusArray[x].usStackHighWaterMark,
                        ulStatsAsPercentage);
            }
            else
            {
                /* 任务运行时间不足总运行时间的1%*/
                sprintf(tmp,
                        "%-15s%-10c%-10lu%-12lu%-12dt<1%%",
                        pxTaskStatusArray[x].pcTaskName,
                        task_state[pxTaskStatusArray[x].eCurrentState],
                        pxTaskStatusArray[x].xTaskNumber,
                        pxTaskStatusArray[x].uxCurrentPriority,
                        pxTaskStatusArray[x].usStackHighWaterMark);
            }
            sBSP_UART_Debug_Printf("%s\n", tmp);
        }
    }
    sBSP_UART_Debug_Printf("任务状态:   r-运行  R-就绪  B-阻塞  S-挂起  D-删除\n");
}

// UART6主机通信测试任务
static sDRV_UART6_HostComm* uart6_comm = nullptr;

// 轮速数据回调函数
void uart6_wheel_speed_callback(const uart6_wheel_speed_t* data)
{
    dbg_printf("[UART6]轮速数据: 左=%.2f, 右=%.2f, 时间戳=%u\n",
               data->left_wheel_speed,
               data->right_wheel_speed,
               (unsigned int)data->timestamp);
}

// 步进电机位置回调函数
void uart6_stepper_pos_callback(const uart6_stepper_pos_t* data)
{
    dbg_printf("[UART6]步进电机位置: 电机1=%ld, 电机2=%ld, 时间戳=%u\n",
               data->stepper1_position,
               data->stepper2_position,
               (unsigned int)data->timestamp);
}

// 组合数据回调函数
void uart6_combined_data_callback(const uart6_combined_data_t* data)
{
    dbg_printf("[UART6]组合数据: 轮速[左:%.2f,右:%.2f] 步进电机[1:%ld,2:%ld]\n",
               data->wheel_data.left_wheel_speed,
               data->wheel_data.right_wheel_speed,
               data->stepper_data.stepper1_position,
               data->stepper_data.stepper2_position);
}

// 错误回调函数
void uart6_error_callback(uart6_error_t error, const char* message)
{
    dbg_printf("[UART6]错误: 代码=%d, 消息=%s\n", error, message);
}

void sAPP_Tasks_UART6_HostCommTest(void* param)
{
    dbg_printf("=== UART6主机通信测试任务启动 ===\n");

    // 创建UART6驱动实例
    uart6_comm = new sDRV_UART6_HostComm(115200, 2000);
    if (uart6_comm == nullptr)
    {
        dbg_printf("[ERROR]创建UART6驱动实例失败\n");
        vTaskDelete(NULL);
        return;
    }

    // 初始化驱动
    if (uart6_comm->init() != 0)
    {
        dbg_printf("[ERROR]UART6驱动初始化失败\n");
        delete uart6_comm;
        uart6_comm = nullptr;
        vTaskDelete(NULL);
        return;
    }

    // 设置回调函数
    uart6_comm->set_wheel_speed_callback(uart6_wheel_speed_callback);
    uart6_comm->set_stepper_pos_callback(uart6_stepper_pos_callback);
    uart6_comm->set_combined_data_callback(uart6_combined_data_callback);
    uart6_comm->set_error_callback(uart6_error_callback);

    // 开始接收数据
    if (uart6_comm->start_receive() != 0)
    {
        dbg_printf("[ERROR]开始接收数据失败\n");
        delete uart6_comm;
        uart6_comm = nullptr;
        vTaskDelete(NULL);
        return;
    }

    dbg_printf("[INFO]UART6驱动初始化完成，开始接收数据\n");
    dbg_printf("[INFO]请通过UART6发送以下测试数据包:\n");
    dbg_printf("  轮速数据: {cmd:wheel,lw:1.5,rw:2.0}\n");
    dbg_printf("  步进电机: {cmd:stepper,s1:1000,s2:-500}\n");
    dbg_printf("  组合数据: {cmd:combined,lw:1.2,rw:1.8,s1:800,s2:200}\n");
    dbg_printf("  心跳包: {cmd:heartbeat}\n");

    uint32_t last_stats_time     = 0;
    uint32_t last_heartbeat_time = 0;

    // 任务主循环
    for (;;)
    {
        uint32_t current_time = HAL_GetTick();

        // 每10秒打印一次统计信息
        if (current_time - last_stats_time >= 10000)
        {
            const uart6_statistics_t* stats = uart6_comm->get_statistics();
            if (stats->total_packets > 0)
            {
                dbg_printf("[UART6]统计信息: 总包=%u, 有效=%u, 错误=%u, 超时=%u\n",
                           (unsigned int)stats->total_packets,
                           (unsigned int)stats->valid_packets,
                           (unsigned int)stats->error_packets,
                           (unsigned int)stats->timeout_errors);
            }

            // 检查数据有效性
            bool data_valid = uart6_comm->is_data_valid(5000);
            dbg_printf("[UART6]数据有效性: %s\n", data_valid ? "有效" : "无效");

            last_stats_time = current_time;
        }

        // 每30秒发送一次心跳包
        if (current_time - last_heartbeat_time >= 30000)
        {
            uart6_comm->send_heartbeat();
            dbg_printf("[UART6]发送心跳包\n");
            last_heartbeat_time = current_time;
        }

        // 检查是否有新的轮速数据
        const uart6_wheel_speed_t* wheel_data = uart6_comm->get_wheel_speed_data();
        if (wheel_data->valid && (current_time - wheel_data->timestamp) < 1000)
        {
            // 这里可以添加对轮速数据的处理逻辑
            // 例如：控制电机、更新显示等
        }

        // 检查是否有新的步进电机数据
        const uart6_stepper_pos_t* stepper_data = uart6_comm->get_stepper_pos_data();
        if (stepper_data->valid && (current_time - stepper_data->timestamp) < 1000)
        {
            // 这里可以添加对步进电机数据的处理逻辑
            // 例如：控制步进电机移动到指定位置
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);   // 1秒循环
    }
}
