#pragma once

#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <forward_list>


#include "sBSP_UART.h"









class sBinOutDrv{
public:
    //输出回调
    using sBD_OutputCb_t = void(*)(GPIO_TypeDef* group,uint16_t pin,GPIO_PinState lv);
    //获取滴答定时器回调
    using sBD_GetTick_t = uint32_t(*)();

    enum class dev_mode_t{
        FOREVER_LOW       = 0, //永远为低电平
        FOREVER_HIGH      = 1, //永远为高电平
        SYMMETRIC_TOGGLE  = 2, //对称周期翻转
        ASYMMETRIC_TOGGLE = 3, //非对称周期翻转
        PULSE_LOW         = 4, //低电平脉冲模式,不触发时为高
        PULSE_HIGH        = 5, //高电平脉冲模式,不触发时为低
    };


    sBinOutDrv();
    ~sBinOutDrv();

    void init();

    void regOutputCb(sBD_OutputCb_t reg_out_ev_cb);
    // void regInputCb (sBD_InputEventCb_t reg_in_ev_cb);
    void regGetTick(sBD_GetTick_t _get_tick);
    int addDev(GPIO_TypeDef* _group,uint16_t _pin,uint16_t _id);
    int addDev(GPIO_TypeDef* _group,uint16_t _pin,uint16_t _id,bool _is_active);
    int confDevMode(uint16_t _id,dev_mode_t _mode,bool _is_default_high);
    int confTime(uint16_t _id,uint32_t _period,uint32_t _high_period);


    
    void handler();




private:
    
    typedef struct{
        GPIO_TypeDef* group;
        uint16_t pin;
    }gpio_t;

    typedef struct{
        uint32_t period;        //时长
        uint32_t high_period;   //高电平时长
        uint32_t next_toggle;   //下一次翻转的时间戳
    }timestamp_t;

    typedef struct{
        gpio_t gpio;        //GPIO
        timestamp_t ts;     //时间戳
        bool is_active;     //是否激活
        bool is_reverse;    //是否翻转输出
        bool is_init_high;  //是否初始输出高
        bool out_level;     //需要输出的电平
        bool is_need_change;//电平是否变化
        bool now_level;     //当前电平
        uint16_t id;        //ID
        dev_mode_t mode;    //设备模式
    }device_t;

    //存储器件的单向链表
    std::forward_list<device_t> list;


    //输出回调
    sBD_OutputCb_t out_cb;
    //获取滴答定时器回调
    sBD_GetTick_t get_tick;

    

    void mode_process(device_t& _dev);
    void symmetricToggleProcess(device_t& _dev);

};





