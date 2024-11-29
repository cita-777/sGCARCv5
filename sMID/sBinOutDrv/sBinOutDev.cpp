#include "sBinOutDev.hpp"


/**
 * sBinOutDev.cpp
 * 二值输出设备
 * 
 * v1.0 241129
 * 目前只支持STM32HAL库
 */



sBinOutDrv::sBinOutDrv(){

}

sBinOutDrv::~sBinOutDrv(){

}

void sBinOutDrv::init(){

}

void sBinOutDrv::regOutputCb(sBD_OutputCb_t reg_out_ev_cb){
    this->out_cb = reg_out_ev_cb;
}

void sBinOutDrv::regGetTick(sBD_GetTick_t _get_tick){
    this->get_tick = _get_tick;
}


//添加设备
int sBinOutDrv::addDev(GPIO_TypeDef* _group,uint16_t _pin,uint16_t _id){
    device_t dev;
    memset(&dev,0,sizeof(device_t));
    dev ={
        .gpio.group = _group,
        .gpio.pin = _pin,
        .id = _id,
        .is_active = true,
        .is_reverse = false,
        .is_init_high = false,
    };

    list.push_front(dev);

    return 0;
}

int sBinOutDrv::addDev(GPIO_TypeDef* _group,uint16_t _pin,uint16_t _id,bool _is_active){
    device_t dev;
    memset(&dev,0,sizeof(device_t));
    dev ={
        .gpio.group = _group,
        .gpio.pin = _pin,
        .id = _id,
        .is_active = _is_active,
        .is_reverse = false,
        .is_init_high = false,
    };

    list.push_front(dev);

    return 0;
}


//配置设备模式
int sBinOutDrv::confDevMode(uint16_t _id,dev_mode_t _mode,bool _is_default_high){
    //根据id找到设备,并且配置
    for(auto& dev : list){
        if(dev.id == _id){
            dev.mode = _mode;
            dev.is_init_high = _is_default_high;
            //配置默认电平
            if(out_cb != nullptr){
                out_cb(dev.gpio.group,dev.gpio.pin,(GPIO_PinState)dev.is_init_high);
            }
            //配置完成
            return 0;
        }
    }
    //没有找到设备
    return -1;
}

//配置时间
int sBinOutDrv::confTime(uint16_t _id,uint32_t _period,uint32_t _high_period){
    //根据id找到设备,并且配置
    for(auto& dev : list){
        if(dev.id == _id){
            dev.ts.period = _period;
            dev.ts.high_period = _high_period;
            //配置完成
            return 0;
        }
    }
    //没有找到设备
    return -1;
}

//模式处理
void sBinOutDrv::mode_process(device_t& _dev){
    //处理永远为LOW的情况
    if(_dev.mode == dev_mode_t::FOREVER_LOW){
        _dev.out_level = 0; //低电平
    }
    //永远高
    else if(_dev.mode == dev_mode_t::FOREVER_HIGH){
        _dev.out_level = 1; //高电平
    }
    //对称翻转
    else if(_dev.mode == dev_mode_t::SYMMETRIC_TOGGLE){
        //如果不需要变化电平了,说明上次已经翻转了,这次需要设置下一次的时间了
        if(_dev.is_need_change == false){
            //设置下一次翻转的时间
            _dev.ts.next_toggle = get_tick() + _dev.ts.period;
            //通知toggle函数,需要翻转了
            _dev.is_need_change = true;
        }else{
            symmetricToggleProcess(_dev);
        }
    }
    //非对称翻转
    else if(_dev.mode == dev_mode_t::ASYMMETRIC_TOGGLE){
        
    }
}

//输出变化处理
void sBinOutDrv::symmetricToggleProcess(device_t& _dev){
    //如果当前时间大于等于下一个要变化的时间则变化
    if(get_tick() >= _dev.ts.next_toggle){
        //翻转电平
        _dev.now_level = !_dev.now_level;
        out_cb(_dev.gpio.group,_dev.gpio.pin,(GPIO_PinState)_dev.now_level);
        _dev.is_need_change = false;
        sBSP_UART_Debug_Printf("id=%u,now=%u,Toggle\n",_dev.id,_dev.now_level);
    }
}

void sBinOutDrv::handler(){
    //遍历所有器件
    for(auto& dev : list){
        //只处理使能的
        if(dev.is_active == true){
            mode_process(dev);
        }
    }
}







