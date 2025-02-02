#include "sAPP_GUI.hpp"


/**
 * sAPP_GUI.cpp
 * 所有的Menu都在此文件里创建
 * 
 * v1.1 241227 bySightseer.
 * 使用构建者模式重写了GUI
 * 
 * v1.2 250201 bySightseer.
 * 基于sLIttleMenu v1.6重构
 * 
 * 
 * 
 */

//引用sLM菜单命名空间
using namespace sLM;


/*各部分UI初始化的声明*/
static void imu_menu_init(ItemBase* parent);
static void feram_menu_init(ItemBase* parent);
static void pwrlight_menu_init(ItemBase* parent);
static void sys_setting_init(ItemBase* parent);
static void battary_info_init(ItemBase* parent);

/*参数更新和事件响应的声明*/
static int int_val_show_update(uint32_t id);
static float sys_update(uint32_t id);
static float battary_info_update(uint32_t id);
static float imu_data_update(uint32_t id);
static float imu_bias_update(uint32_t id);

static void button_event_cb(ItemBase* item,uint32_t id);
static void int_val_adj_event_cb(IntValAdj* item,uint32_t id,int value);


#define ID_SYS_SETTINGS                             1
#define ID_IMU                                      2
#define ID_BATTARY_INFO                             3
#define ID_BALANCE_CTRL                             4
#define ID_POWER_LIGHT                              5

#define ID_SYS_SYSCLOCK                             6
#define ID_SYS_MCUTEMP                              7
#define ID_SYS_VCCVOLT                              8
#define ID_SYS_FERAMSETTINGS                        9
#define ID_SYS_FERAMSETTINGS_CLEAR                  10

#define ID_SYS_BATTARYINFO_VOLTAGE                  11
#define ID_SYS_BATTARYINFO_CURRENT                  12
#define ID_SYS_BATTARYINFO_POWER                    13

#define ID_IMU_DATAOVERVIEW                         14
#define ID_IMU_PITCH                                15
#define ID_IMU_ROLL                                 16
#define ID_IMU_YAW                                  17
#define ID_IMU_ACCX                                 18
#define ID_IMU_ACCY                                 19
#define ID_IMU_ACCZ                                 20
#define ID_IMU_GYRX                                 21
#define ID_IMU_GYRY                                 22
#define ID_IMU_GYRZ                                 23
#define ID_IMU_MAGX                                 24
#define ID_IMU_MAGY                                 25
#define ID_IMU_MAGZ                                 26
#define ID_IMU_ICMTEMP                              27
#define ID_IMU_LIS3TEMP                             28

#define ID_IMU_ZEROBIAS                             29
#define ID_IMU_BIAS_AX                              30
#define ID_IMU_BIAS_AY                              31
#define ID_IMU_BIAS_AZ                              32
#define ID_IMU_BIAS_GX                              33
#define ID_IMU_BIAS_GY                              34
#define ID_IMU_BIAS_GZ                              35
#define ID_IMU_CALIBIAS                             36

void sAPP_GUI_Init(){
    //初始化菜单,传入渲染器
    menu.init(new OLED128X64(&oled,&menu));
    
    /*初始化所有UI组件*/

    /*系统设置菜单*/
    auto* sys_settings = &EnterableItem::create(menu.getHome(),ID_SYS_SETTINGS).setTittle("Sys settings");
    
    /*IMU*/
    auto* imu = &EnterableItem::create(menu.getHome(),ID_IMU).setTittle("IMU");
    /*battary info*/
    auto* battary_info = &EnterableItem::create(menu.getHome(),ID_BATTARY_INFO).setTittle("Battary info");
    /*balance ctrl*/
    ButtonItem::create(menu.getHome(),ID_BALANCE_CTRL).setContext("Balance ctrl","PRESS").setCallback(button_event_cb);
    /*power light*/
    IntValAdj::create(menu.getHome(),ID_POWER_LIGHT)
        .setCallback(int_val_adj_event_cb,CallBackMethod::CHANGE)
        .setContext("Power Light","%d%%",0,20,20)
        .setConstraint(ConstraintType::RANGE,100,0);

    //创建里面的菜单项
    sys_setting_init(sys_settings);
    battary_info_init(battary_info);
    imu_menu_init(imu);
}

static void sys_setting_init(ItemBase* parent){
    /*系统主频*/
    FloatValShow::create(parent,ID_SYS_SYSCLOCK)
        .setContext("Sys clock","%.0fMHz")
        .setCallback(sys_update);
    /*MCU温度*/
    FloatValShow::create(parent,ID_SYS_MCUTEMP)
        .setContext("MCU temp","%.1fdegC")
        .setCallback(sys_update);
    /*MCU电压*/
    FloatValShow::create(parent,ID_SYS_VCCVOLT)
        .setContext("VCC voltage","%.3fv")
        .setCallback(sys_update);

    /*FeRAM*/
    auto* feram_settings = &EnterableItem::create(parent,ID_SYS_FERAMSETTINGS)
        .setTittle("FeRAM settings");

    ButtonItem::create(feram_settings,ID_SYS_FERAMSETTINGS_CLEAR)
        .setContext("Clear to 0","CLEAR")
        .setCallback(button_event_cb);
}


static void battary_info_init(ItemBase* parent){
    /*电池电压*/
    FloatValShow::create(parent,ID_SYS_BATTARYINFO_VOLTAGE)
        .setContext("Voltage","%.2fV")
        .setCallback(battary_info_update);
    FloatValShow::create(parent,ID_SYS_BATTARYINFO_CURRENT)
        .setContext("Current","%.3fA")
        .setCallback(battary_info_update);
    FloatValShow::create(parent,ID_SYS_BATTARYINFO_POWER)
        .setContext("Power","%.2fW")
        .setCallback(battary_info_update);
}


static void imu_menu_init(ItemBase* parent){
    /*查看IMU的数据 可进入的项*/
    auto* data_overview = &EnterableItem::create(parent,ID_IMU_DATAOVERVIEW)
        .setTittle("Real-time Data");

    /*俯仰角 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_PITCH).setContext("Pitch","%.2f").setCallback(imu_data_update);
    /*横滚角 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_ROLL).setContext("Roll","%.2f").setCallback(imu_data_update);
    /*航向角 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_YAW).setContext("Yaw","%.2f").setCallback(imu_data_update);
    /*Accel_X 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_ACCX).setContext("Acc-X","%.2f").setCallback(imu_data_update);
    /*Accel_Y 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_ACCY).setContext("Acc-Y","%.2f").setCallback(imu_data_update);
    /*Accel_Z 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_ACCZ).setContext("Acc-Z","%.2f").setCallback(imu_data_update);
    /*Gyro_X 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_GYRX).setContext("Gyro-X","%.2f").setCallback(imu_data_update);
    /*Gyro_Y 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_GYRY).setContext("Gyro-Y","%.2f").setCallback(imu_data_update);
    /*Gryo_Z 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_GYRZ).setContext("Gyro-Z","%.2f").setCallback(imu_data_update);
    /*Mag-X 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_MAGX).setContext("Mag-X",nullptr).setCallback(imu_data_update);
    /*Mag-Y 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_MAGY).setContext("Mag-Y",nullptr).setCallback(imu_data_update);
    /*Mag-Z 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_MAGZ).setContext("Mag-Z",nullptr).setCallback(imu_data_update);
    /*ICM Temp 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_ICMTEMP).setContext("ICM-Temp",nullptr).setCallback(imu_data_update);
    /*LIS3 Temp 只读数据*/
    FloatValShow::create(data_overview,ID_IMU_LIS3TEMP).setContext("LIS3-Temp",nullptr).setCallback(imu_data_update);

    /*0偏页面*/
    auto* zero_bias = &EnterableItem::create(parent,ID_IMU_ZEROBIAS).setTittle("Zero-bias");

    /*BiasAX 只读数据*/
    FloatValShow::create(zero_bias,ID_IMU_BIAS_AX).setContext("Bias Acc-X","%.3f").setCallback(imu_bias_update);
    /*BiasAY 只读数据*/
    FloatValShow::create(zero_bias,ID_IMU_BIAS_AY).setContext("Bias Acc-Y","%.3f").setCallback(imu_bias_update);
    /*BiasAZ 只读数据*/
    FloatValShow::create(zero_bias,ID_IMU_BIAS_AZ).setContext("Bias Acc-Z","%.3f").setCallback(imu_bias_update);
    /*BiasGX 只读数据*/
    FloatValShow::create(zero_bias,ID_IMU_BIAS_GX).setContext("Bias Gyro-X","%.3f").setCallback(imu_bias_update);
    /*BiasGY 只读数据*/
    FloatValShow::create(zero_bias,ID_IMU_BIAS_GY).setContext("Bias Gyro-Y","%.3f").setCallback(imu_bias_update);
    /*BiasGZ 只读数据*/
    FloatValShow::create(zero_bias,ID_IMU_BIAS_GZ).setContext("Bias Gyro-Z","%.3f").setCallback(imu_bias_update);
    /*0偏校准 是一个按钮*/
    ButtonItem::create(zero_bias,ID_IMU_CALIBIAS).setContext("Calibrate","START").setCallback(button_event_cb);
}

static float imu_data_update(uint32_t id){
    float tmp = 0;
    if(xSemaphoreTake(ahrs.mutex,20) == pdTRUE){
        if(id == ID_IMU_PITCH){
            tmp = ahrs.dat.pitch;
        }
        else if(id == ID_IMU_ROLL){
            tmp = ahrs.dat.roll;
        }
        else if(id == ID_IMU_YAW){
            tmp = ahrs.dat.yaw;
        }
        else if(id == ID_IMU_ACCX){
            tmp = ahrs.dat.acc_x;
        }
        else if(id == ID_IMU_ACCY){
            tmp = ahrs.dat.acc_y;
        }
        else if(id == ID_IMU_ACCZ){
            tmp = ahrs.dat.acc_z;
        }
        else if(id == ID_IMU_GYRX){
            tmp = ahrs.dat.gyr_x;
        }
        else if(id == ID_IMU_GYRY){
            tmp = ahrs.dat.gyr_y;
        }
        else if(id == ID_IMU_GYRZ){
            tmp = ahrs.dat.gyr_z;
        }
        else if(id == ID_IMU_MAGX){
            tmp = ahrs.dat.mag_x;
        }
        else if(id == ID_IMU_MAGY){
            tmp = ahrs.dat.mag_y;
        }
        else if(id == ID_IMU_MAGZ){
            tmp = ahrs.dat.mag_z;
        }
        else if(id == ID_IMU_ICMTEMP){
            tmp = ahrs.icm_temp;
        }
        else if(id == ID_IMU_LIS3TEMP){
            tmp = ahrs.lis3_temp;
        }
        xSemaphoreGive(ahrs.mutex);
    }
    return tmp;
}

static float imu_bias_update(uint32_t id){
    float tmp = 0;
    if(id == ID_IMU_BIAS_AX){
        tmp = ahrs.imu_sbias.acc_x;
    }
    else if(id == ID_IMU_BIAS_AY){
        tmp = ahrs.imu_sbias.acc_y;
    }
    else if(id == ID_IMU_BIAS_AZ){
        tmp = ahrs.imu_sbias.acc_z;
    }
    else if(id == ID_IMU_BIAS_GX){
        tmp = ahrs.imu_sbias.gyr_x;
    }
    else if(id == ID_IMU_BIAS_GY){
        tmp = ahrs.imu_sbias.gyr_y;
    }
    else if(id == ID_IMU_BIAS_GZ){
        tmp = ahrs.imu_sbias.gyr_z;
    }
    return tmp;
}


static int int_val_show_update(uint32_t id){
    int tmp = 0;

    return tmp;
}

static float sys_update(uint32_t id){
    float tmp = 0;
    if(xSemaphoreTake(car.mutex,200) == pdTRUE){
        if(id == ID_SYS_SYSCLOCK){
            tmp = (float)car.coreClock / 1e6;
        }
        else if(id == ID_SYS_MCUTEMP){
            tmp = car.mcu_temp;
        }
        else if(id == ID_SYS_VCCVOLT){
            tmp = car.mcu_volt;
        }
        xSemaphoreGive(car.mutex);
    }
    return tmp;
}

static float battary_info_update(uint32_t id){
    float tmp = 0;
    if(xSemaphoreTake(car.mutex,200) == pdTRUE){
        if(id == ID_SYS_BATTARYINFO_VOLTAGE){
            tmp = car.batt_volt;
        }
        else if(id == ID_SYS_BATTARYINFO_CURRENT){
            tmp = car.batt_curr;
        }
        else if(id == ID_SYS_BATTARYINFO_POWER){
            tmp = car.batt_power;
        }
        xSemaphoreGive(car.mutex);
    }
    return tmp;
}

static void button_event_cb(ItemBase* item,uint32_t id){
    if(id == ID_BALANCE_CTRL){
        g_ctrl.blc_en = !g_ctrl.blc_en;
    }
    else if(id == ID_SYS_FERAMSETTINGS_CLEAR){
        sAPP_Tasks_StartFormatFeRAM();
    }
    else if(id == ID_IMU_CALIBIAS){
        sAPP_Tasks_StartCalibrateIMU();
    }
}

static void int_val_adj_event_cb(IntValAdj* item,uint32_t id,int value){
    if(id == ID_POWER_LIGHT){
        sDRV_PL_SetBrightness((float)value);
    }
}





