#include "sAPP_Car.hpp"

#include "main.h"


sAPP_Car car;




sAPP_Car::sAPP_Car() {}

sAPP_Car::~sAPP_Car() {}




int sAPP_Car::initSys()
{
    // 初始化HAL库
    HAL_Init();
    // 初始化RCC
    sBSP_RCC_Init();
    // 初始化SysTick的TIM6的优先级为4
    HAL_InitTick(4);
    // 获取时钟频率
    coreClock = HAL_RCC_GetSysClockFreq();
    // 初始化DWT
    dwt.init(coreClock);
    // 初始化RNG
    sBSP_RNG_Init();
    // 初始化NTC读取温度
    sDRV_NTC_Init();
    // 初始化调试串口
    sBSP_UART_Debug_Init(115200);
    // sBSP_UART_Debug_Init(512000);

    // 初始化顶层通信串口
    // sBSP_UART_Top_Init(115200);
    // 初始化ADC
    sBSP_ADC_Init();

    // 启用除0异常
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
    // 初始化cm_backtrace崩溃调试
    cm_backtrace_init(APPNAME, HARDWARE_VERSION, SOFTWARE_VERSION);

    return 0;
}


int sAPP_Car::initBoard()
{
    HAL_Delay(10);   // 等待上电稳定

    sBSP_I2C1_Init(400000);

    // 初始化电机
    motor.init();
    // 初始化按键
    sAPP_Btns_Init();
    // 初始化二值输出设备:蜂鸣器,LED
    sAPP_BOD_Init();
    // 大功率灯初始化
    sDRV_PL_Init();
    // 初始化屏幕
    sDRV_GenOLED_Init();
    // 初始化OLED
    oled.init();
    // 初始化铁电
    sAPP_ParamSave_Init();
    // 初始化航姿参考系统
    ahrs.init(AHRS::IMUType::ICM45686, AHRS::MAGType::LIS3MDLTR);
    // 初始化INA219
    sDRV_INA219::CONFIG_t config = {
        .bus_adc    = sDRV_INA219::ADC_SETTING::SET_128SAMPLES_68D1MS,
        .bvr        = sDRV_INA219::BUS_VOLTAGE_RANGE::RANGE_16V,
        .pga        = sDRV_INA219::PGA_GAIN::GAIN_2_80MV,
        .shunt_adc  = sDRV_INA219::ADC_SETTING::SET_128SAMPLES_68D1MS,
        .mode       = sDRV_INA219::MODE::SHUNT_AND_BUS_VOLTAGE_CONTINUOUS,
        .rshunt_ohm = 0.05,   // 50mR
    };



    if (ina.init(&config) != 0)
    {
        sBSP_UART_Debug_Printf("INA219初始化失败!\n");
        return -1;
    }

    // 创建car的数据的互斥锁
    mutex = xSemaphoreCreateMutex();

    // OLED清屏
    oled.setAll(0);
    oled.handler();

    return 0;
}


extern "C" void sAPP_Car_InfoUpdateTask(void* param)
{
    for (;;)
    {

        ina.getData();
        vTaskDelay(200);


        if (xSemaphoreTake(car.mutex, 200) == pdTRUE)
        {
            car.coreClock = HAL_RCC_GetHCLKFreq();
            car.mcu_temp  = sBSP_ADC_GetMCUTemp();
            car.mcu_volt  = sBSP_ADC_GetVCC();

            //! 这里应该先获取I2C1总线的互斥锁,这里只先测试
            car.batt_volt  = ina.getBusV();
            car.batt_curr  = ina.getCurrA();
            car.batt_power = ina.getPowerW();
            xSemaphoreGive(car.mutex);
        }


        // sBSP_UART_Debug_Printf("%.2f,%.2f,%.2f\n", car.batt_volt, car.batt_curr,car.batt_power);
        // sBSP_UART_Debug_Printf("up!\n");
    }
}
