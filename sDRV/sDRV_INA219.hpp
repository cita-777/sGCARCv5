#pragma once


#include "sBSP_I2C.h"


class sDRV_INA219{
public:
    enum class BUS_VOLTAGE_RANGE{
        RANGE_16V = 0,
        RANGE_32V = 1,      //默认
    };
    enum class PGA_GAIN{
        GAIN_1_40MV  = 0b00,
        GAIN_2_80MV  = 0b01,
        GAIN_4_160MV = 0b10,
        GAIN_8_320MV = 0b11,   //默认
    };
    enum class ADC_SETTING{
        SET_9BIT_84US         = 0b0000,
        SET_10BIT_148US       = 0b0001,
        SET_11BIT_276US       = 0b0010,
        SET_12BIT_532US       = 0b0011,   //默认
        SET_2SAMPLES_1D06MS   = 0b1000,
        SET_4SAMPLES_2D13MS   = 0b1010,
        SET_8SAMPLES_4D26MS   = 0b1011,
        SET_16SAMPLES_8D51MS  = 0b1100,
        SET_32SAMPLES_17D02MS = 0b1101,
        SET_64SAMPLES_34D05MS = 0b1110,
        SET_128SAMPLES_68D1MS = 0b1111,
    };
    enum class MODE{
        POWER_DOWN                      = 0b000,
        SHUNT_VOLTAGE_TRIGGERED         = 0b001,
        BUS_VOLTAGE_TRIGGERED           = 0b010,
        SHUNT_AND_BUS_VOLTAGE_TRIGGERED = 0b011,
        ADC_OFF                         = 0b100,
        SHUNT_VOLTAGE_CONTINUOUS        = 0b101,
        BUS_VOLTAGE_CONTINUOUS          = 0b110,
        SHUNT_AND_BUS_VOLTAGE_CONTINUOUS= 0b111, //默认
    };
    struct CONFIG_t{
        BUS_VOLTAGE_RANGE bvr;
        PGA_GAIN pga;
        ADC_SETTING shunt_adc;
        ADC_SETTING bus_adc;
        MODE mode;
        float rshunt_ohm;
    };

    inline static constexpr uint8_t DEFAULT_ADDR = 0x40 << 1; // INA219 I2C地址
    //默认RSHUNT电阻值
    inline static constexpr float DEFAULT_RSHUNT_OHM = 0.050f; // 50mR

    sDRV_INA219(){}
    ~sDRV_INA219(){}

    int init(CONFIG_t* config,uint8_t dev_addr = DEFAULT_ADDR);
    int init(uint8_t dev_addr = DEFAULT_ADDR);

    int getData();

    float getCurrA(){return current;}
    float getRshuntV(){return shunt_voltage;}
    float getBusV(){return bus_voltage;}
    float getPowerW(){return power;}

private:
    //INA219寄存器地址
    inline static constexpr uint8_t ADDR_CONFIG        = 0x00;
    inline static constexpr uint8_t ADDR_SHUNT_VOLTAGE = 0x01;
    inline static constexpr uint8_t ADDR_BUS_VOLTAGE   = 0x02;
    inline static constexpr uint8_t ADDR_CALIBRATION   = 0x05;
    inline static constexpr uint8_t ADDR_CURRENT       = 0x04;
    inline static constexpr uint8_t ADDR_POWER         = 0x03;


    //所有INA219共用的接口
    static inline void write_reg(uint8_t dev_addr,uint8_t reg_addr,uint16_t data){
        HAL_I2C_Mem_Write(&hi2c1, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&data, 2, 1000);
    }
    static inline uint16_t read_reg(uint8_t dev_addr,uint8_t reg_addr){
        uint16_t data = 0;
        HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&data, 2, 1000);
        return data;
    }
    static inline void read_regs(uint8_t dev_addr,uint8_t reg_addr, uint16_t* pData, uint8_t len){
        HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)pData, len, 1000);
    }
    static inline bool dev_is_ready(uint8_t dev_addr){
        return sBSP_I2C1M_DevIsReady(dev_addr);
    }

    uint8_t dev_addr; // INA219 I2C地址
    float rshunt_ohm = DEFAULT_RSHUNT_OHM; // RSHUNT电阻值
    float shunt_voltage = 0.0f; // 分流电压
    float bus_voltage = 0.0f; // 总线电压
    float current = 0.0f; // 电流
    float power = 0.0f; // 功率
};



extern sDRV_INA219 ina;


