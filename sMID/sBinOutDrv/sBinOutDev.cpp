#include "sBinOutDev.hpp"


/**
 * sBinOutDev.cpp
 * 二值输出设备
 * 
 * v1.0 241130 bySightseer. inHNIP9607Lab
 * 目前只支持STM32HAL库
 * 在PULSE_LOW模式时,有bug,在初始时状态不对,过一会儿就没问题了
 * 
 */

/**
 * 使用说明
 * 1.首先实例化对象并初始化:
 * sBinOutDrv bod;
 * bod.init();
 * 
 * 2.添加一个设备,指明GPIO口,注意设备要先被初始化:
 * bod.addDev(LED_ID,GPIOC,GPIO_PIN_13);
 * 
 * 3.配置设备模式,注意指明初始电平
 * bod.confDevMode(LED_ID,sBOD::DEV_MODE::SYMMETRIC_TOGGLE,sBOD::LEVEL::LOW);
 * 
 * 4.如何使用(FOREVER_LOW/HIGH模式比较简单,忽略)
 *  1)使用SYMMETRIC_TOGGLE(对称翻转)模式:
 *      在对称翻转模式下,_high_period参数无效
 *      bod.confTime(LED_ID,1000,50);
 *  2)使用ASYMMETRIC_TOGGLE(非对称翻转)模式:
 *      配置波形周期1000ms,高电平周期50ms
 *      bod.confTime(LED_ID,1000,50);
 *  3)使用PULSE_LOW(低电平脉冲)模式:
 *      配置低电平脉宽50ms
 *      bod.confTime(LED_ID,1000,50);
 *  4)使用PULSE_HIGH(高电平脉冲)模式:
 *      配置高电平脉宽50ms
 *      bod.confTime(LED_ID,1000,50);
 * 
 * 5.周期性调用处理函数,注意时间粒度取决于调用间隔
 *      bod.update();
 * 
 * 
 */


sBinOutDrv BinOutDrv;


sBinOutDrv::sBinOutDrv(){

}

sBinOutDrv::~sBinOutDrv(){

}

void sBinOutDrv::init(){
    //把list清空
    memset(list, 0, sizeof(list));
}

//输出
inline void sBinOutDrv::output(uint16_t id,GPIO_TypeDef* group,uint16_t pin,GPIO_PinState lv){
    HAL_GPIO_WritePin(group,pin,(GPIO_PinState)lv);
    //sBSP_UART_Debug_Printf("id=%u,lv=%u,tick=%u\n",id,lv,getTick());
}

//获取滴答定时器
inline uint32_t sBinOutDrv::getTick(){
    return HAL_GetTick();
}

void sBinOutDrv::devGPIOInit(GPIO_TypeDef* group,uint16_t pin){
    __GPIOA_CLK_ENABLE();__GPIOB_CLK_ENABLE();__GPIOC_CLK_ENABLE();__GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Pin   = pin;
    HAL_GPIO_Init(group,&gpio);
}

//添加设备,传入GPIO组,引脚,器件ID号
int sBinOutDrv::addDev(uint16_t _id,GPIO_TypeDef* _group,uint16_t _pin){
    //合法性检查
    if(_id > SBOD_MAX_DEVICES || _group == nullptr){return -1;}

    list[_id].gpio.group   = _group;
    list[_id].gpio.pin     = _pin;
    list[_id].is_activate  = true;  //默认激活
    list[_id].init_level   = LEVEL::LOW;

    //初始化GPIO
    devGPIOInit(_group,_pin);

    return 0;
}

//添加设备,传入GPIO组,引脚,器件ID号,是否激活
int sBinOutDrv::addDev(uint16_t _id,GPIO_TypeDef* _group,uint16_t _pin,bool _is_activate){
    //合法性检查
    if(_id > SBOD_MAX_DEVICES || _group == nullptr){return -1;}

    list[_id].gpio.group   = _group;
    list[_id].gpio.pin     = _pin;
    list[_id].is_activate  = _is_activate;
    list[_id].init_level   = LEVEL::LOW;
    
    //初始化GPIO
    devGPIOInit(_group,_pin);
    
    return 0;
}

//配置设备模式,传入要配置的器件的id,模式,是否默认为高电平
int sBinOutDrv::confDevMode(uint16_t _id,DEV_MODE _mode,LEVEL _init_level){
    //合法性检查
    if(_id > SBOD_MAX_DEVICES){return -1;}

    //根据id找到设备,并且配置
    list[_id].mode = _mode;
    list[_id].init_level = _init_level;
    //模式已改变,需要变化电平
    list[_id].is_need_change = true;
    //配置默认电平
    output(_id,list[_id].gpio.group,list[_id].gpio.pin,(GPIO_PinState)list[_id].init_level);
    return 0;
}

//配置时间,传入id,时间,高电平时间
int sBinOutDrv::confTime(uint16_t _id,uint32_t _period,uint32_t _high_period){
    //合法性检查
    if(_id > SBOD_MAX_DEVICES){return -1;}
    if(_high_period > _period){
        return -2;
    }

    list[_id].ts.period = _period;
    list[_id].ts.high_period = _high_period;
    return 0;
}

int sBinOutDrv::startPulse(uint16_t _id){
    //合法性检查
    if(_id > SBOD_MAX_DEVICES){return -1;}
    DEVICE& dev = list[_id];
    if(dev.mode != DEV_MODE::PULSE_LOW && dev.mode != DEV_MODE::PULSE_HIGH){
        return -2;
    }

    if(dev.mode == DEV_MODE::PULSE_LOW){
        dev.now_level = LEVEL::HIGH;
        dev.pulse_count = 0;
    }
    else if(dev.mode == DEV_MODE::PULSE_HIGH){
        dev.now_level = LEVEL::LOW;
        dev.pulse_count = 0;
    }
    return 0;
}

int sBinOutDrv::startPulse(uint16_t _id,uint32_t period){
    //合法性检查
    if(_id > SBOD_MAX_DEVICES){return -1;}
    DEVICE& dev = list[_id];
    if(dev.mode != DEV_MODE::PULSE_LOW && dev.mode != DEV_MODE::PULSE_HIGH){
        return -2;
    }

    dev.ts.period = period;
    dev.ts.high_period = period;

    if(dev.mode == DEV_MODE::PULSE_LOW){
        dev.now_level = LEVEL::HIGH;
        dev.pulse_count = 0;
    }
    else if(dev.mode == DEV_MODE::PULSE_HIGH){
        dev.now_level = LEVEL::LOW;
        dev.pulse_count = 0;
    }
    return 0;
}



void sBinOutDrv::foreverLowProcess(uint16_t id){
    //如果需要改变电平,则调用处理函数处理
    if(list[id].is_need_change == true){
        output(id,list[id].gpio.group,list[id].gpio.pin,(GPIO_PinState)LEVEL::LOW);
        list[id].is_need_change = false; //已经改变了,下次不改变了
        list[id].now_level = LEVEL::LOW; //告诉模块当前是低电平
    }
}

void sBinOutDrv::foreverHighProcess(uint16_t id){
    //如果需要改变电平,则调用处理函数处理
    if(list[id].is_need_change == true){
        output(id,list[id].gpio.group,list[id].gpio.pin,(GPIO_PinState)LEVEL::HIGH);
        list[id].is_need_change = false; //已经改变了,下次不改变了
        list[id].now_level = LEVEL::HIGH;//告诉模块当前是高电平
    }
}

//对称翻转处理
void sBinOutDrv::symmetricToggleProcess(uint16_t id){
    DEVICE& dev = list[id];
    //如果当前时间-上一次变化的时间 >= 周期时间 就说明超时了,该翻转电平了
    if(getTick() - dev.ts.prev_toggle >= dev.ts.period){
        //更新上一次变化时间
        dev.ts.prev_toggle = getTick();
        //状态取反
        dev.now_level = (LEVEL)!(bool)dev.now_level;
        //设置输出
        output(id,dev.gpio.group,dev.gpio.pin,(GPIO_PinState)dev.now_level);
    }
}

//非对称翻转处理
void sBinOutDrv::asymmetricToggleProcess(uint16_t id){
    DEVICE& dev = list[id];
    //和对称翻转大体一样原理,只是多了根据器件的状态选择比较哪个时间
    if((getTick() - dev.ts.prev_toggle) >= \
    (dev.now_level == LEVEL::HIGH ? dev.ts.high_period : dev.ts.period - dev.ts.high_period)){
        dev.ts.prev_toggle = getTick();
        dev.now_level = (LEVEL)!(bool)dev.now_level;
        output(id,dev.gpio.group,dev.gpio.pin,(GPIO_PinState)dev.now_level);
    }
}

//低电平脉冲模式处理
void sBinOutDrv::pulseLowProcess(uint16_t id){
    DEVICE& dev = list[id];
    //这个标志位使用来保证只进入两次单脉冲模式的,进入两次是一个完整脉冲
    if(dev.pulse_count >= 2){return;}
    //同上,只是只会触发2次
    if((getTick() - dev.ts.prev_toggle) >= \
    (dev.now_level == LEVEL::LOW ? dev.ts.high_period : dev.ts.period - dev.ts.high_period)){
        dev.ts.prev_toggle = getTick();
        dev.now_level = (LEVEL)!(bool)dev.now_level;
        output(id,dev.gpio.group,dev.gpio.pin,(GPIO_PinState)dev.now_level);
        dev.pulse_count++; //计数器++
    }
}

//高电平脉冲模式处理
void sBinOutDrv::pulseHighProcess(uint16_t id){
    DEVICE& dev = list[id];
    //这个标志位使用来保证只进入两次单脉冲模式的,进入两次是一个完整脉冲
    if(dev.pulse_count >= 2){return;}
    //同上,只是只会触发2次
    if((getTick() - dev.ts.prev_toggle) >= \
    (dev.now_level == LEVEL::HIGH ? dev.ts.high_period : dev.ts.period - dev.ts.high_period)){
        dev.ts.prev_toggle = getTick();
        dev.now_level = (LEVEL)!(bool)dev.now_level;
        output(id,dev.gpio.group,dev.gpio.pin,(GPIO_PinState)dev.now_level);
        dev.pulse_count++; //计数器++
    }
}

//模式处理
void sBinOutDrv::mode_process(uint16_t id){
    DEVICE& dev = list[id];
    //处理永远为LOW的情况
    if(dev.mode == DEV_MODE::FOREVER_LOW){
        foreverLowProcess(id);
    }
    //永远高
    else if(dev.mode == DEV_MODE::FOREVER_HIGH){
        foreverHighProcess(id);
    }
    //对称翻转
    else if(dev.mode == DEV_MODE::SYMMETRIC_TOGGLE){
        symmetricToggleProcess(id);
    }
    //非对称翻转
    else if(dev.mode == DEV_MODE::ASYMMETRIC_TOGGLE){
        asymmetricToggleProcess(id);
    }
    //低电平脉冲模式
    else if(dev.mode == DEV_MODE::PULSE_LOW){
        pulseLowProcess(id);
    }
    //高电平脉冲模式
    else if(dev.mode == DEV_MODE::PULSE_HIGH){
        pulseHighProcess(id);
    }
}

//实测只需要3us左右就能完成
void sBinOutDrv::update(){
    //遍历所有器件
    for(uint16_t id = 0; id < SBOD_MAX_DEVICES;id++){
        //只处理使能的
        if(list[id].is_activate == true){
            mode_process(id);
        }
    }
}




