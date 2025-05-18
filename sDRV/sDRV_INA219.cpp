#include "sDRV_INA219.hpp"


#include "sightseerUtils\sUtils.h"



sDRV_INA219 ina;


int sDRV_INA219::init(uint8_t dev_addr){
    //默认配置
    CONFIG_t default_config;
    CONFIG_t* config = &default_config;
    config->bus_adc = ADC_SETTING::SET_12BIT_532US;
    config->bvr = BUS_VOLTAGE_RANGE::RANGE_32V;
    config->pga = PGA_GAIN::GAIN_8_320MV;
    config->shunt_adc = ADC_SETTING::SET_12BIT_532US;
    config->mode = MODE::SHUNT_AND_BUS_VOLTAGE_CONTINUOUS;
    config->rshunt_ohm = DEFAULT_RSHUNT_OHM;

    init(config,dev_addr); // 调用带配置的初始化函数

    return 0;
}

int sDRV_INA219::init(CONFIG_t* config,uint8_t dev_addr){
    if(!config){
        return -1;
    }

    //首先确保通信正常
    if(!dev_is_ready(dev_addr)){
        return -2; // 设备未响应
    }

    //然后重置INA219寄存器数据
    write_reg(dev_addr, ADDR_CONFIG, 0x8000); // 重置寄存器
    HAL_Delay(10); // 等待重置完成

    //配置INA219寄存器
    uint16_t config_data = 0;
    config_data |= (static_cast<uint16_t>(config->bvr) << 13); // 设置电压范围
    config_data |= (static_cast<uint16_t>(config->pga) << 11); // 设置增益
    config_data |= (static_cast<uint16_t>(config->shunt_adc) << 7); // 设置分流电压ADC设置
    config_data |= (static_cast<uint16_t>(config->bus_adc) << 3); // 设置总线电压ADC设置
    config_data |=  static_cast<uint16_t>(config->mode); // 设置工作模式
    //交换字节序
    config_data = (config_data << 8) | (config_data >> 8);
    write_reg(dev_addr, ADDR_CONFIG, config_data);

    this->rshunt_ohm = config->rshunt_ohm; // 设置分流电阻值

    //配置校准寄存器
    uint16_t calibration = static_cast<uint16_t>(0.04096f / (rshunt_ohm * 0.001f)); //1mA/LSB
    //交换字节序
    calibration = (calibration << 8) | (calibration >> 8);
    write_reg(dev_addr, ADDR_CALIBRATION, calibration);

    this->dev_addr = dev_addr; // 设置设备地址

    HAL_Delay(10); // 等待配置完成

    return 0;
}

int sDRV_INA219::getData(){
    /*读回数据*/
    uint16_t rawShuntVoltage = read_reg(dev_addr, ADDR_SHUNT_VOLTAGE);
    uint16_t rawCurrent      = read_reg(dev_addr, ADDR_CURRENT);
    uint16_t rawPower        = read_reg(dev_addr, ADDR_POWER);
    uint16_t rawConfig       = read_reg(dev_addr, ADDR_CONFIG);
    uint16_t rawCalibration  = read_reg(dev_addr, ADDR_CALIBRATION);
    uint16_t rawBusVoltage    = read_reg(dev_addr, ADDR_BUS_VOLTAGE);
    /*交换大小端*/
    rawShuntVoltage          = (rawShuntVoltage << 8) | (rawShuntVoltage >> 8);
    rawCurrent               = (rawCurrent      << 8) | (rawCurrent      >> 8);
    rawPower                 = (rawPower        << 8) | (rawPower        >> 8);
    rawConfig                = (rawConfig       << 8) | (rawConfig       >> 8);
    rawCalibration           = (rawCalibration  << 8) | (rawCalibration  >> 8);
    rawBusVoltage            = (rawBusVoltage   << 8) | (rawBusVoltage   >> 8);

    /*解析数据*/
    //总线电压
    //屏蔽掉低3位
    rawBusVoltage >>= 3;
    //LSB为4mV/bit
    bus_voltage = rawBusVoltage * 0.004f; //单位V
    //电流,直接读出就是mA,因为配置了1mA/LSB
    current = (static_cast<int16_t>(rawCurrent) * 0.001f); //单位A
    //功率,LSB=20mW/bit
    power = (static_cast<int16_t>(rawPower) * 0.02f); //单位W
    //分流电压,LSB=10uV/bit
    shunt_voltage = (static_cast<int16_t>(rawShuntVoltage) * 0.01f) / 1000.0f; //单位V


    return 0;
}



