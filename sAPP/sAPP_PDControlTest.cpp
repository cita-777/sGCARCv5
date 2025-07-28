#include "sAPP_PDControlTest.hpp"
#include "main.h"
#include "sAPP_AHRS.hpp"
#include "sAPP_Motor.hpp"
#include "sBSP_UART.h"
#include "sDRV_GMR.h"
#include <math.h>

// PD控制测试任务
void sAPP_Tasks_PDControlTest(void* param)
{
    sBSP_UART_Debug_Printf("[INFO] PD航向控制系统测试任务启动\n");

    // 等待系统初始化完成
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // 初始化电机系统
    motor.init();
    motor.enableClosedLoopControl(true);

    // 设置PID参数 - 左右电机分别调整
    motor.setLeftPIDParams(0.6f, 1.2f, 0.0f);    // 左电机参数
    motor.setRightPIDParams(0.6f, 1.2f, 0.0f);   // 右电机参数稍高（因为右轮容易超调）

    // 设置PD航向控制参数 - 极低参数避免震荡
    motor.setHeadingPDParams(1.0f, 0.0f);

    sBSP_UART_Debug_Printf("[INFO] 电机系统初始化完成，开始PD控制测试\n");

    // 控制周期参数
    const float CONTROL_DT    = 0.02f;   // 20ms控制周期，50Hz
    TickType_t  xLastWakeTime = xTaskGetTickCount();

    // 测试状态机
    enum TestState
    {
        TEST_INIT,
        TEST_HEADING_HOLD,
        TEST_STRAIGHT_LINE,
        TEST_TURN_TO_ANGLE,
        TEST_DISTANCE_CONTROL,
        TEST_DISTANCE_WITH_HEADING,
        TEST_COMPLETE
    };

    TestState current_test = TEST_INIT;
    uint32_t  test_timer   = 0;
    uint32_t  print_timer  = 0;

    for (;;)
    {
        // 更新电机编码器数据
        motor.update();

        // 执行闭环控制更新
        motor.updateClosedLoopControl(CONTROL_DT);

        test_timer++;
        print_timer++;

        // 每秒打印一次状态信息
        if (print_timer >= 50)
        {   // 50 * 20ms = 1秒
            print_timer = 0;

            float current_heading  = motor.getCurrentHeading();
            float target_heading   = motor.getTargetHeading();
            float heading_error    = motor.getHeadingError();
            float current_distance = motor.getCurrentDistance();
            float target_distance  = motor.getTargetDistance();
            float distance_error   = motor.getDistanceError();

            float left_target_rpm  = motor.getLeftTargetRPM();
            float right_target_rpm = motor.getRightTargetRPM();
            float left_actual_rpm  = motor.getLRPM();
            float right_actual_rpm = motor.getRRPM();

            sBSP_UART_Debug_Printf("%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
                                   current_heading,
                                   target_heading,
                                   left_target_rpm,
                                   left_actual_rpm,
                                   right_target_rpm,
                                   right_actual_rpm);
        }

        // 测试状态机
        switch (current_test)
        {
        case TEST_INIT:
            sBSP_UART_Debug_Printf("[TEST] 开始测试1: 航向保持测试\n");
            motor.resetDistance();
            motor.setTargetHeading(0.0f);       // 保持0度航向
            motor.enableHeadingControl(true);   // 启用航向控制保持直行
            motor.setTargetRPM(15.0f, 15.0f);   // 降低到15 RPM前进
            current_test = TEST_HEADING_HOLD;
            test_timer   = 0;
            break;

        case TEST_HEADING_HOLD:
            // 持续测试直行，不切换到其他测试
            // if (test_timer >= 250)
            // {   // 250 * 20ms = 5秒
            //     motor.stop();
            //     vTaskDelay(1000 / portTICK_PERIOD_MS);
            //     sBSP_UART_Debug_Printf("[TEST] 开始测试2: 直线行驶测试\n");
            //     motor.moveForwardWithHeading(20.0f, 0.0f);
            //     current_test = TEST_STRAIGHT_LINE;
            //     test_timer   = 0;
            // }
            break;

        // 注释掉其他测试，专注于直行调试
        /*
        case TEST_STRAIGHT_LINE:
        case TEST_TURN_TO_ANGLE:
        case TEST_DISTANCE_CONTROL:
        case TEST_DISTANCE_WITH_HEADING:
        case TEST_COMPLETE:
        */
        default:
            // 其他状态暂时不处理
            break;
        }

        // 精确的20ms周期控制
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

// 简化的PD控制演示任务
void sAPP_Tasks_SimplePDDemo(void* param)
{
    sBSP_UART_Debug_Printf("[INFO] 简化PD控制演示任务启动\n");

    // 等待系统初始化
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // 初始化电机
    motor.init();
    motor.enableClosedLoopControl(true);
    motor.setPIDParams(0.8f, 0.1f, 0.05f);
    motor.setHeadingPDParams(0.5f, 0.1f);

    const float CONTROL_DT    = 0.02f;
    TickType_t  xLastWakeTime = xTaskGetTickCount();

    uint32_t demo_counter = 0;

    for (;;)
    {
        motor.update();
        motor.updateClosedLoopControl(CONTROL_DT);

        demo_counter++;

        // 每10秒切换一个演示动作
        uint32_t demo_phase = (demo_counter / 500) % 4;   // 500 * 20ms = 10秒

        switch (demo_phase)
        {
        case 0:
            // 直线前进，保持0度航向
            if (demo_counter % 500 == 0)
            {
                sBSP_UART_Debug_Printf("[DEMO] 直线前进，保持0度航向\n");
                motor.moveForwardWithHeading(30.0f, 0.0f);
            }
            break;

        case 1:
            // 转向到90度
            if (demo_counter % 500 == 0)
            {
                sBSP_UART_Debug_Printf("[DEMO] 转向到90度\n");
                motor.turnToHeading(90.0f);
            }
            break;

        case 2:
            // 前进1米
            if (demo_counter % 500 == 0)
            {
                sBSP_UART_Debug_Printf("[DEMO] 前进1米\n");
                motor.resetDistance();
                motor.moveDistance(30.0f, 1.0f);
            }
            break;

        case 3:
            // 停止
            if (demo_counter % 500 == 0)
            {
                sBSP_UART_Debug_Printf("[DEMO] 停止\n");
                motor.stop();
            }
            break;
        }

        // 每秒打印状态
        if (demo_counter % 50 == 0)
        {
            sBSP_UART_Debug_Printf("航向:%.1f/%.1f, 距离:%.2f/%.2f, 完成:%d\n",
                                   motor.getCurrentHeading(),
                                   motor.getTargetHeading(),
                                   motor.getCurrentDistance(),
                                   motor.getTargetDistance(),
                                   motor.isMovementComplete());
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}
