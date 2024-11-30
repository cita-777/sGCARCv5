#pragma once

#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sBSP_UART.h"


#define SBOD_MAX_DEVICES 8






class sBinOutDrv{
public:

    enum class DEV_MODE{
        FOREVER_LOW       = 0, //永远为低电平
        FOREVER_HIGH      = 1, //永远为高电平
        SYMMETRIC_TOGGLE  = 2, //对称周期翻转
        ASYMMETRIC_TOGGLE = 3, //非对称周期翻转
        PULSE_LOW         = 4, //低电平脉冲模式,不触发时为高
        PULSE_HIGH        = 5, //高电平脉冲模式,不触发时为低
    };

    enum class LEVEL{
        LOW = 0,
        HIGH = 1,
    };

    




    sBinOutDrv();
    ~sBinOutDrv();

    void init();

    int addDev(uint16_t _id,GPIO_TypeDef* _group,uint16_t _pin);
    int addDev(uint16_t _id,GPIO_TypeDef* _group,uint16_t _pin,bool _is_activate);
    int confDevMode(uint16_t _id,DEV_MODE _mode,LEVEL _init_level);
    int confTime(uint16_t _id,uint32_t _period,uint32_t _high_period);

    int startPulse(uint16_t _id);


    
    void update();


private:
    //GPIO
    typedef struct{
        GPIO_TypeDef* group;
        uint16_t pin;
    }GPIO;
    //时间戳
    typedef struct{
        uint32_t period;        //时长
        uint32_t high_period;   //高电平时长
        uint32_t prev_toggle;   //上一次翻转的时间戳
        uint32_t next_toggle;   //下一次翻转的时间戳
    }TIMESTAMP;
    //器件
    typedef struct{
        GPIO gpio;        //GPIO
        TIMESTAMP ts;     //时间戳
        bool is_activate;     //是否激活
        LEVEL init_level;  //初始输出电平
        LEVEL out_level;     //需要输出的电平
        bool is_need_change;//电平是否变化
        LEVEL now_level;     //用于跟踪当前电平
        DEV_MODE mode;    //设备模式
        uint16_t pulse_count; //脉冲计数
    
    }DEVICE;

    //存储器件的数组
    DEVICE list[SBOD_MAX_DEVICES];

    
    void output(uint16_t id,GPIO_TypeDef* group,uint16_t pin,GPIO_PinState lv);
    uint32_t getTick();
    void devGPIOInit(GPIO_TypeDef* group,uint16_t pin);

    void mode_process(uint16_t id);

    void foreverLowProcess(uint16_t id);
    void foreverHighProcess(uint16_t id);
    void symmetricToggleProcess(uint16_t id);
    void asymmetricToggleProcess(uint16_t id);
    void pulseLowProcess(uint16_t id);
    void pulseHighProcess(uint16_t id);

};


//给sBinOutDev起一个别名sBOD
using sBOD = sBinOutDrv;


extern sBinOutDrv BinOutDrv;


