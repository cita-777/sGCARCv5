#include "sBinOutDev.hpp"




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


void sBinOutDrv::addDev(GPIO_TypeDef* _group,uint16_t _pin,uint16_t _id,sBD_DevDir_t _dir){
    sBinOutDrv_t drv = {.group = _group, .pin = _pin, .dir = _dir};
    this->list[_id] = drv;
    this->list[_id].is_need_change = true;
    this->list[_id].enable = true;
    
}

void sBinOutDrv::confDevMode(uint16_t _id,sBD_DevMode_t _mode,bool _is_default_high){
    this->list[_id].mode = _mode;
    this->list[_id].is_default_high = _is_default_high;

    out_cb(list[_id].group,list[_id].pin,(sBD_Level_t)list[_id].is_default_high);
    list[_id].now_level = (sBD_Level_t)list[_id].is_default_high;
}

void sBinOutDrv::confTime(uint16_t _id,uint32_t _period,uint32_t _high_period){
    list[_id].tog_period = _period;
    list[_id].high_period = _high_period;
}




void sBinOutDrv::mode_process(uint16_t _id){
    //处理永远为LOW的情况
    if(this->list[_id].mode == sBD_DevMode_t::FOREVER_LOW){
        list[_id].out_level = sBD_Level_t::LOW;
    }
    //HIGH
    else if(this->list[_id].mode == sBD_DevMode_t::FOREVER_HIGH){
        list[_id].out_level = sBD_Level_t::HIGH;
    }
    //对称翻转
    else if(list[_id].mode == sBD_DevMode_t::SYMMETRIC_TOGGLE){
        //如果不需要变化电平了,说明上次已经翻转了,这次需要设置下一次的时间了
        if(list[_id].is_need_change == false){
            //设置下一次翻转的时间
            list[_id].next_tog_ts = get_tick() + list[_id].tog_period;
            //通知toggle函数,需要翻转了
            list[_id].is_need_change = true;
        }else{
            symmetricToggleProcess(_id);
        }
    }
    //非对称翻转
    else if(list[_id].mode == sBD_DevMode_t::ASYMMETRIC_TOGGLE){
        
    }
}

//输出变化处理
void sBinOutDrv::symmetricToggleProcess(uint16_t _id){
    //如果当前时间大于等于下一个要变化的时间则变化
    if(get_tick() >= this->list[_id].next_tog_ts){
        list[_id].now_level = (sBD_Level_t)!(bool)list[_id].now_level;
        out_cb(list[_id].group,list[_id].pin,(sBD_Level_t)list[_id].now_level);
        list[_id].is_need_change = false;
        sBSP_UART_Debug_Printf("id=%u,now=%u,Toggle\n",_id,list[_id].now_level);
    }

    
}

void sBinOutDrv::handler(){
    //遍历所有器件
    for(uint16_t i = 0;i < SBD_MAX_DEVICES_NUM;i++){
        //只处理使能的
        if(this->list[i].enable == true){
            mode_process(i);
        }
    }
}







