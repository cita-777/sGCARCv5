#pragma once

#include "stm32f4xx_hal.h"

#include "sBSP_UART.h"


#define SBD_MAX_DEVICES_NUM 4

enum class sBD_Level_t{
    LOW = 0,
    HIGH = 1,
};

enum class sBD_DevDir_t{
    OUTPUT = 0,
    INPUT  = 1,
};

enum class sBD_DevMode_t{
    FOREVER_LOW       = 0, //永远为低电平
    FOREVER_HIGH      = 1, //永远为高电平
    SYMMETRIC_TOGGLE  = 2, //对称周期翻转
    ASYMMETRIC_TOGGLE = 3, //非对称周期翻转
    PULSE_LOW         = 4, //低电平脉冲模式,不触发时为高
    PULSE_HIGH        = 5, //高电平脉冲模式,不触发时为低
};






//输出回调
using sBD_OutputCb_t = void(*)(GPIO_TypeDef* group,uint16_t pin,sBD_Level_t lv);
//输入回调
// using sBD_InputCb_t  = void(*)(GPIO_TypeDef* group,uint16_t pin,sBD_Level_t lv);

using sBD_GetTick_t = uint32_t(*)();




class sBinOutDrv{
public:
    sBinOutDrv();
    ~sBinOutDrv();

    void init();

    void regOutputCb(sBD_OutputCb_t reg_out_ev_cb);
    // void regInputCb (sBD_InputEventCb_t reg_in_ev_cb);
    void regGetTick(sBD_GetTick_t _get_tick);
    void addDev(GPIO_TypeDef* _group,uint16_t _pin,uint16_t _id,sBD_DevDir_t _dir);
    void confDevMode(uint16_t _id,sBD_DevMode_t _mode,bool _is_default_high);
    void confTime(uint16_t _id,uint32_t _period,uint32_t _high_period);


    
    void handler();




private:
    typedef struct{
        GPIO_TypeDef* group;
        uint16_t pin;
        sBD_DevMode_t mode;
        sBD_DevDir_t  dir;
        bool is_need_change;
        bool enable;
        uint32_t tog_period;
        uint32_t high_period;
        bool is_default_high;
        sBD_Level_t out_level;
        sBD_Level_t now_level;
    
        uint32_t next_tog_ts;
    }sBinOutDrv_t;

    sBinOutDrv_t list[SBD_MAX_DEVICES_NUM];
    sBD_OutputCb_t out_cb;
    sBD_GetTick_t get_tick;

    

    void mode_process(uint16_t _id);
    void symmetricToggleProcess(uint16_t _id);

};





