#include "sAPP_GUI.hpp"


/**
 * sAPP_GUI.cpp
 * 所有的Menu都在此文件里创建
 * 
 * v1.1 241227 bySightseer.
 * 使用构建者模式重写了GUI
 * 
 * 
 * 
 * 
 */

//引用sLM菜单命名空间
using namespace sLM;

/*TAG 用于告诉回调读取要显示的参数是哪个*/
#define TAG_IMU_PITCH      1
#define TAG_IMU_ROLL       2
#define TAG_IMU_YAW        3
#define TAG_IMU_ACC_X      4
#define TAG_IMU_ACC_Y      5
#define TAG_IMU_ACC_Z      6
#define TAG_IMU_GYR_X      7
#define TAG_IMU_GYR_Y      8
#define TAG_IMU_GYR_Z      9
#define TAG_IMU_MAG_X      10
#define TAG_IMU_MAG_Y      11
#define TAG_IMU_MAG_Z      12


#define TAG_CALI_IMU       13
#define TAG_CLEAR_FERAM    14
#define TAG_BLC_CTRL       15

#define TAG_IMU_BIAS_AX    16
#define TAG_IMU_BIAS_AY    17
#define TAG_IMU_BIAS_AZ    18
#define TAG_IMU_BIAS_GX    19
#define TAG_IMU_BIAS_GY    20
#define TAG_IMU_BIAS_GZ    21

#define TAG_SYS_SYS_CLOCK  22
#define TAG_SYS_MCU_TEMP   23
#define TAG_SYS_VCC_VOLT   24

#define TAG_BAT_VOLT       25
#define TAG_BAT_CURR       26
#define TAG_BAT_POWER      27




/*各部分UI初始化的声明*/
static void imu_menu_init(TreeNode* parent);
static void feram_menu_init(TreeNode* parent);
static void pwrlight_menu_init(TreeNode* parent);
static void sys_setting_init(TreeNode* parent);
static void battary_info_init(TreeNode* parent);

/*参数更新和事件响应的声明*/
static void imu_data_update(void* param,uint32_t _tag);
static void imu_bias_update(void* param,uint32_t _tag);
static void sys_settings_update(void* param,uint32_t _tag);
static void battary_info_update(void* param,uint32_t _tag);

static ParamModifyLock button_event_cb(void* param,ParamType _type);
static ParamModifyLock pwrlight_event_cb(void* param,ParamType _type);




void sAPP_GUI_Init(){
    menu.init(new OLED128X64(&oled,&menu));
    
    /*初始化所有UI组件*/

    /*系统设置菜单 可进入的项*/
    TreeNode* sys_setting_item = CreateItem().setText("Sys Settings").create();
    menu.addSubMenu(menu.getRoot(),sys_setting_item);

    /*Motor Info*/
    TreeNode* motor_info = CreateItem().setText("Motor Info").create();
    menu.addSubMenu(menu.getRoot(),motor_info);


    /*IMU菜单*/
    TreeNode* imu_item = CreateItem().setText("IMU Settings").create();
    menu.addSubMenu(menu.getRoot(),imu_item);

    /*Battary Info*/
    TreeNode* battary_info = CreateItem().setText("Battary Info").create();
    menu.addSubMenu(menu.getRoot(),battary_info);


    /**/



    sys_setting_init(sys_setting_item);
    imu_menu_init(imu_item);
    battary_info_init(battary_info);
    // feram_menu_init(menu.getRoot());


    pwrlight_menu_init(menu.getRoot());


    // 创建一个整数类型的菜单项
    // TreeNode* intItem = CreateItem()
    //     .setText("Int Para").setUnit("u")
    //     .setParamTag(TAG_IMU_YAW)
    //     .setAccess(ParamAccess::RO)
    //     .setParamType(ParamType::FLOAT)  
    //     .setItemType(ItemType::NORMAL) 
    //     .setChangeCallback(nullptr)
    //     .setUpdateCallback(imu_data_update) 
    //     .setChangeMethod(ParamModifyCbMethod::EXIT_CB) 
    //     .setLimitType(ParamLimitType::NO) 
    //     .setValue(42.0f).setMax(100.0f).setMin(0.0f)
    //     .setIncrement(10.2f).setDecrement(10.2f)
    //     .create();
    // menu.addSubMenu(menu.getRoot(),intItem);

    // TreeNode* parent = menu.getRoot();
    // for(int i = 2; i <= 100; ++i){
    //     std::string menuName = "Menu " + std::to_string(i);
    //     TreeNode* child = CreateItem().setText(menuName.c_str()).create();
    //     menu.addSubMenu(parent, child);                                                                                                                                                                                                                   
    //     parent = child;
    // }


}

static void sys_setting_init(TreeNode* parent){
    /*系统主频*/
    menu.addSubMenu(parent,CreateItem()
        .setText("Sys Clock").setUnit("MHz").setParamTag(TAG_SYS_SYS_CLOCK)
        .setAccess(ParamAccess::RO).setParamType(ParamType::INT).setUpdateCallback(sys_settings_update).create());

    /*MCU温度*/
    menu.addSubMenu(parent,CreateItem()
        .setText("MCU Temp").setUnit("degC").setParamTag(TAG_SYS_MCU_TEMP).setIntParamShowFmt("%.1f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(sys_settings_update).create());

    /*MCU电压*/
    menu.addSubMenu(parent,CreateItem()
        .setText("MCU VCC").setUnit("v").setParamTag(TAG_SYS_VCC_VOLT).setIntParamShowFmt("%.3f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(sys_settings_update).create());

    
    /*FeRAM*/
    TreeNode* feram_settings = CreateItem().setText("FeRAM Settings").create();
    menu.addSubMenu(parent,feram_settings);

    menu.addSubMenu(feram_settings,CreateItem()
        .setText("Set All to 0").setUnit("")
        .setParamTag(TAG_CLEAR_FERAM).setItemType(ItemType::BUTTON).setChangeCallback(button_event_cb).create());

    

}


static void battary_info_init(TreeNode* parent){
    /*电池电压*/
    menu.addSubMenu(parent,CreateItem()
        .setText("Voltage").setUnit("V").setParamTag(TAG_BAT_VOLT).setIntParamShowFmt("%.2f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(battary_info_update).create());
    
    menu.addSubMenu(parent,CreateItem()
        .setText("Current").setUnit("A").setParamTag(TAG_BAT_CURR).setIntParamShowFmt("%.3f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(battary_info_update).create());

    menu.addSubMenu(parent,CreateItem()
        .setText("Power").setUnit("W").setParamTag(TAG_BAT_POWER).setIntParamShowFmt("%.2f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(battary_info_update).create());

}


static void imu_menu_init(TreeNode* parent){
    /*查看IMU的数据 可进入的项*/
    TreeNode* data_overview = CreateItem().setText("Real-time Data").create();
    menu.addSubMenu(parent,data_overview);

    /*俯仰角 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Pitch").setUnit("").setParamTag(TAG_IMU_PITCH)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*横滚角 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Roll").setUnit("").setParamTag(TAG_IMU_ROLL)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*航向角 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Yaw").setUnit("").setParamTag(TAG_IMU_YAW)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*Accel_X 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Accel-X").setUnit("").setParamTag(TAG_IMU_ACC_X)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*Accel_Y 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Accel-Y").setUnit("").setParamTag(TAG_IMU_ACC_Y)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*Accel_Z 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Accel-Z").setUnit("").setParamTag(TAG_IMU_ACC_Z)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*Gyro_X 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Gyro-X").setUnit("").setParamTag(TAG_IMU_GYR_X)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*Gyro_Y 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Gyro-Y").setUnit("").setParamTag(TAG_IMU_GYR_Y)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*Gryo_Z 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Gyro-Z").setUnit("").setParamTag(TAG_IMU_GYR_Z)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());

    /*Mag-X 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Mag-X").setUnit("").setParamTag(TAG_IMU_MAG_X)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*Mag-Y 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Mag-Y").setUnit("").setParamTag(TAG_IMU_MAG_Y)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());
    /*Mag-Z 只读数据*/
    menu.addSubMenu(data_overview,CreateItem()
        .setText("Mag-Z").setUnit("").setParamTag(TAG_IMU_MAG_Z)
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_data_update).create());

    /*0偏页面*/
    TreeNode* zero_bias = CreateItem().setText("Zero-Bias").create();
    menu.addSubMenu(parent,zero_bias);

    /*BiasAX 只读数据*/
    menu.addSubMenu(zero_bias,CreateItem()
        .setText("Bias Acc-X").setUnit("").setParamTag(TAG_IMU_BIAS_AX).setIntParamShowFmt("%.4f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_bias_update).create());
    /*BiasAY 只读数据*/
    menu.addSubMenu(zero_bias,CreateItem()
        .setText("Bias Acc-Y").setUnit("").setParamTag(TAG_IMU_BIAS_AY).setIntParamShowFmt("%.4f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_bias_update).create());
    /*BiasAZ 只读数据*/
    menu.addSubMenu(zero_bias,CreateItem()
        .setText("Bias Acc-Z").setUnit("").setParamTag(TAG_IMU_BIAS_AZ).setIntParamShowFmt("%.4f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_bias_update).create());
    /*BiasGX 只读数据*/
    menu.addSubMenu(zero_bias,CreateItem()
        .setText("Bias Gyro-X").setUnit("").setParamTag(TAG_IMU_BIAS_GX).setIntParamShowFmt("%.4f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_bias_update).create());
    /*BiasGY 只读数据*/
    menu.addSubMenu(zero_bias,CreateItem()
        .setText("Bias Gyro-Y").setUnit("").setParamTag(TAG_IMU_BIAS_GY).setIntParamShowFmt("%.4f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_bias_update).create());
    /*BiasGZ 只读数据*/
    menu.addSubMenu(zero_bias,CreateItem()
        .setText("Bias Gyro-Z").setUnit("").setParamTag(TAG_IMU_BIAS_GZ).setIntParamShowFmt("%.4f%s")
        .setAccess(ParamAccess::RO).setParamType(ParamType::FLOAT).setUpdateCallback(imu_bias_update).create());

    /*0偏校准 是一个按钮*/
    menu.addSubMenu(zero_bias,CreateItem()
        .setText("Bias Calibrate").setUnit("")
        .setParamTag(TAG_CALI_IMU).setItemType(ItemType::BUTTON).setChangeCallback(button_event_cb).create());


}



static void pwrlight_menu_init(TreeNode* parent){
    menu.addSubMenu(parent,CreateItem()
        .setText("PowerLight").setUnit("%")
        .setAccess(ParamAccess::RW).setParamType(ParamType::INT).setItemType(ItemType::NORMAL) 
        .setChangeCallback(pwrlight_event_cb).setChangeMethod(ParamModifyCbMethod::CHRG_CB) 
        .setLimitType(ParamLimitType::RANGE)
        .setValue(0).setMax(100).setMin(0)
        .setIncrement(10).setDecrement(10)
        .create());
}

static void imu_data_update(void* param,uint32_t _tag){
    if(xSemaphoreTake(ahrs.mutex,20) == pdTRUE){
        if(_tag == TAG_IMU_PITCH){
            *(float*)param = ahrs.dat.pitch;
        }
        else if(_tag == TAG_IMU_ROLL){
            *(float*)param = ahrs.dat.roll;
        }
        else if(_tag == TAG_IMU_YAW){
            *(float*)param = ahrs.dat.yaw;
        }
        else if(_tag == TAG_IMU_ACC_X){
            *(float*)param = ahrs.dat.acc_x;
        }
        else if(_tag == TAG_IMU_ACC_Y){
            *(float*)param = ahrs.dat.acc_y;
        }
        else if(_tag == TAG_IMU_ACC_Z){
            *(float*)param = ahrs.dat.acc_z;
        }
        else if(_tag == TAG_IMU_GYR_X){
            *(float*)param = ahrs.dat.gyr_x;
        }
        else if(_tag == TAG_IMU_GYR_Y){
            *(float*)param = ahrs.dat.gyr_y;
        }
        else if(_tag == TAG_IMU_GYR_Z){
            *(float*)param = ahrs.dat.gyr_z;
        }
        else if(_tag == TAG_IMU_MAG_X){
            *(float*)param = ahrs.dat.mag_x;
        }
        else if(_tag == TAG_IMU_MAG_Y){
            *(float*)param = ahrs.dat.mag_y;
        }
        else if(_tag == TAG_IMU_MAG_Z){
            *(float*)param = ahrs.dat.mag_z;
        }

        xSemaphoreGive(ahrs.mutex);
    }
    
}

static void imu_bias_update(void* param,uint32_t _tag){
    if(_tag == TAG_IMU_BIAS_AX){
        *(float*)param = ahrs.imu_sbias.acc_x;
    }
    else if(_tag == TAG_IMU_BIAS_AY){
        *(float*)param = ahrs.imu_sbias.acc_y;
    }
    else if(_tag == TAG_IMU_BIAS_AZ){
        *(float*)param = ahrs.imu_sbias.acc_z;
    }
    else if(_tag == TAG_IMU_BIAS_GX){
        *(float*)param = ahrs.imu_sbias.gyr_x;
    }
    else if(_tag == TAG_IMU_BIAS_GY){
        *(float*)param = ahrs.imu_sbias.gyr_y;
    }
    else if(_tag == TAG_IMU_BIAS_GZ){
        *(float*)param = ahrs.imu_sbias.gyr_z;
    }
}


static void sys_settings_update(void* param,uint32_t _tag){
    if(xSemaphoreTake(car.mutex,200) == pdTRUE){
        if(_tag == TAG_SYS_SYS_CLOCK){
            *(int*)param = car.coreClock / 1e6;
        }
        else if(_tag == TAG_SYS_MCU_TEMP){
            *(float*)param = car.mcu_temp;
        }
        else if(_tag == TAG_SYS_VCC_VOLT){
            *(float*)param = car.mcu_volt;
        }
        xSemaphoreGive(car.mutex);
    }
}

static void battary_info_update(void* param,uint32_t _tag){
    if(xSemaphoreTake(car.mutex,200) == pdTRUE){
        if(_tag == TAG_BAT_VOLT){
            *(float*)param = car.batt_volt;
        }
        else if(_tag == TAG_BAT_CURR){
            *(float*)param = car.batt_curr;
        }
        else if(_tag == TAG_BAT_POWER){
            *(float*)param = car.batt_power;
        }
        xSemaphoreGive(car.mutex);
    }
}

static ParamModifyLock button_event_cb(void* param,ParamType _type){
    if(_type == ParamType::BUTTON_PRESS){
        if(*(uint32_t*)param == TAG_CALI_IMU){
            sAPP_Tasks_StartCalibrateIMU();
            return ParamModifyLock::UNLOCK;
        }
        else if(*(uint32_t*)param == TAG_CLEAR_FERAM){
            sAPP_Tasks_StartFormatFeRAM();
            return ParamModifyLock::LOCK;
        }
        else if(*(uint32_t*)param == TAG_BLC_CTRL){
            g_ctrl.blc_en = !g_ctrl.blc_en;
            return ParamModifyLock::UNLOCK;
        }
    }
    
    return ParamModifyLock::UNLOCK;
}

static ParamModifyLock pwrlight_event_cb(void* param,ParamType _type){
    uint32_t val = *(uint32_t*)param;
    sDRV_PL_SetBrightness((float)val);
    return ParamModifyLock::UNLOCK;
}





