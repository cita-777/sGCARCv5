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

// 引用sLM菜单命名空间
using namespace sLM;


/*各部分UI初始化的声明*/
static void imu_menu_init(ItemBase* parent);
static void feram_menu_init(ItemBase* parent);
static void pwrlight_menu_init(ItemBase* parent);
static void sys_setting_init(ItemBase* parent);
static void battary_info_init(ItemBase* parent);
static void alt_est_info_init(ItemBase* parent);

/*参数更新和事件响应的声明*/
static int         int_val_show_update(uint32_t id);
static const char* string_val_show_update(uint32_t id);
static float       sys_update(uint32_t id);
static float       battary_info_update(uint32_t id);
static float       imu_data_update(uint32_t id);
static float       imu_bias_update(uint32_t id);
static float       alt_est_update(uint32_t id);

static void button_event_cb(ItemBase* item, uint32_t id);
static void int_val_adj_event_cb(IntValAdj* item, uint32_t id, int value);

// 把ID全部变成枚举类
enum ID : std::uint32_t
{
    SYS_SETTINGS = 1,
    IMU,
    BATTARY_INFO,
    BALANCE_CTRL,
    POWER_LIGHT,
    SYS_SYSCLOCK,
    SYS_MCUTEMP,
    SYS_VCCVOLT,
    SYS_FERAMSETTINGS,
    SYS_FERAMSETTINGS_CLEAR,
    SYS_BATTARYINFO_VOLTAGE,
    SYS_BATTARYINFO_CURRENT,
    SYS_BATTARYINFO_POWER,
    IMU_DATAOVERVIEW,
    IMU_PITCH,
    IMU_ROLL,
    IMU_YAW,
    IMU_ACCX,
    IMU_ACCY,
    IMU_ACCZ,
    IMU_GYRX,
    IMU_GYRY,
    IMU_GYRZ,
    IMU_MAGX,
    IMU_MAGY,
    IMU_MAGZ,
    IMU_ICMTEMP,
    IMU_LIS3TEMP,
    IMU_ZEROBIAS,
    IMU_BIAS_AX,
    IMU_BIAS_AY,
    IMU_BIAS_AZ,
    IMU_BIAS_GX,
    IMU_BIAS_GY,
    IMU_BIAS_GZ,
    IMU_CALI_ABIAS,
    IMU_CALI_GBIAS,
    ALT_EST_INFO,
    ALT_EST_ALGO,
    ALT_EST_R,
    ALT_EST_P,
    ALT_EST_CHI,
    ALT_EST_EK,
    ALT_EST_ACC_NORM,
    IMU_IMU_INFO,
    IMU_MAG_INFO,
    IMU_TYPE,
    IMU_STATE,
    IMU_MAG_TYPE,
    IMU_MAG_STATE,

    COUNT,
};




void sAPP_GUI_Init()
{
    // 初始化菜单,传入渲染器
    menu.init(new OLED128X64(&oled, &menu));

    /*初始化所有UI组件*/

    /*系统设置菜单*/
    auto* sys_settings = &EnterableItem::create(menu.getHome(), ID::SYS_SETTINGS).setTittle("Sys settings");

    /*IMU*/
    auto* imu_data = &EnterableItem::create(menu.getHome(), ID::IMU).setTittle("IMU");

    auto* alt_est_info = &EnterableItem::create(menu.getHome(), ID::ALT_EST_INFO).setTittle("AltEst info");
    /*battary info*/
    auto* battary_info = &EnterableItem::create(menu.getHome(), ID::BATTARY_INFO).setTittle("Battary info");
    /*balance ctrl*/
    ButtonItem::create(menu.getHome(), ID::BALANCE_CTRL)
        .setContext("Balance ctrl", "PRESS")
        .setCallback(button_event_cb);
    /*power light*/
    IntValAdj::create(menu.getHome(), ID::POWER_LIGHT)
        .setCallback(int_val_adj_event_cb, CallBackMethod::CHANGE)
        .setContext("Power Light", "%d%%", 0, 20, 20)
        .setConstraint(ConstraintType::RANGE, 100, 0);

    // 创建里面的菜单项
    sys_setting_init(sys_settings);
    battary_info_init(battary_info);
    imu_menu_init(imu_data);

    alt_est_info_init(alt_est_info);
}

static void sys_setting_init(ItemBase* parent)
{
    /*系统主频*/
    FloatValShow::create(parent, ID::SYS_SYSCLOCK).setContext("Sys clock", "%.0fMHz").setCallback(sys_update);
    /*MCU温度*/
    FloatValShow::create(parent, ID::SYS_MCUTEMP).setContext("MCU temp", "%.1fdegC").setCallback(sys_update);
    /*MCU电压*/
    FloatValShow::create(parent, ID::SYS_VCCVOLT).setContext("VCC voltage", "%.3fv").setCallback(sys_update);

    /*FeRAM*/
    auto* feram_settings = &EnterableItem::create(parent, ID::SYS_FERAMSETTINGS).setTittle("FeRAM settings");

    ButtonItem::create(feram_settings, ID::SYS_FERAMSETTINGS_CLEAR)
        .setContext("Clear to 0", "CLEAR")
        .setCallback(button_event_cb);
}


static void battary_info_init(ItemBase* parent)
{
    /*电池电压*/
    FloatValShow::create(parent, ID::SYS_BATTARYINFO_VOLTAGE)
        .setContext("Voltage", "%.2fV")
        .setCallback(battary_info_update);
    FloatValShow::create(parent, ID::SYS_BATTARYINFO_CURRENT)
        .setContext("Current", "%.3fA")
        .setCallback(battary_info_update);
    FloatValShow::create(parent, ID::SYS_BATTARYINFO_POWER)
        .setContext("Power", "%.2fW")
        .setCallback(battary_info_update);
}


static void imu_menu_init(ItemBase* parent)
{
    /*查看IMU的数据 可进入的项*/
    auto* data_overview = &EnterableItem::create(parent, ID::IMU_DATAOVERVIEW).setTittle("Real-time Data");

    /*俯仰角 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_PITCH).setContext("Pitch", "%.2f").setCallback(imu_data_update);
    /*横滚角 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_ROLL).setContext("Roll", "%.2f").setCallback(imu_data_update);
    /*航向角 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_YAW).setContext("Yaw", "%.2f").setCallback(imu_data_update);
    /*Accel_X 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_ACCX).setContext("Acc-X", "%.2f").setCallback(imu_data_update);
    /*Accel_Y 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_ACCY).setContext("Acc-Y", "%.2f").setCallback(imu_data_update);
    /*Accel_Z 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_ACCZ).setContext("Acc-Z", "%.2f").setCallback(imu_data_update);
    /*Gyro_X 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_GYRX).setContext("Gyro-X", "%.2f").setCallback(imu_data_update);
    /*Gyro_Y 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_GYRY).setContext("Gyro-Y", "%.2f").setCallback(imu_data_update);
    /*Gryo_Z 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_GYRZ).setContext("Gyro-Z", "%.2f").setCallback(imu_data_update);
    /*Mag-X 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_MAGX).setContext("Mag-X", nullptr).setCallback(imu_data_update);
    /*Mag-Y 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_MAGY).setContext("Mag-Y", nullptr).setCallback(imu_data_update);
    /*Mag-Z 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_MAGZ).setContext("Mag-Z", nullptr).setCallback(imu_data_update);
    /*ICM Temp 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_ICMTEMP).setContext("ICM-Temp", nullptr).setCallback(imu_data_update);
    /*LIS3 Temp 只读数据*/
    FloatValShow::create(data_overview, ID::IMU_LIS3TEMP).setContext("LIS3-Temp", nullptr).setCallback(imu_data_update);

    /*0偏页面*/
    auto* zero_bias = &EnterableItem::create(parent, ID::IMU_ZEROBIAS).setTittle("Zero-bias");

    /*BiasAX 只读数据*/
    FloatValShow::create(zero_bias, ID::IMU_BIAS_AX).setContext("Bias Acc-X", "%.3f").setCallback(imu_bias_update);
    /*BiasAY 只读数据*/
    FloatValShow::create(zero_bias, ID::IMU_BIAS_AY).setContext("Bias Acc-Y", "%.3f").setCallback(imu_bias_update);
    /*BiasAZ 只读数据*/
    FloatValShow::create(zero_bias, ID::IMU_BIAS_AZ).setContext("Bias Acc-Z", "%.3f").setCallback(imu_bias_update);
    /*BiasGX 只读数据*/
    FloatValShow::create(zero_bias, ID::IMU_BIAS_GX).setContext("Bias Gyro-X", "%.3f").setCallback(imu_bias_update);
    /*BiasGY 只读数据*/
    FloatValShow::create(zero_bias, ID::IMU_BIAS_GY).setContext("Bias Gyro-Y", "%.3f").setCallback(imu_bias_update);
    /*BiasGZ 只读数据*/
    FloatValShow::create(zero_bias, ID::IMU_BIAS_GZ).setContext("Bias Gyro-Z", "%.3f").setCallback(imu_bias_update);
    /*加速度0偏校准 是一个按钮*/
    ButtonItem::create(zero_bias, ID::IMU_CALI_ABIAS).setContext("Calib Accel", "START").setCallback(button_event_cb);
    /*陀螺仪0偏校准 是一个按钮*/
    ButtonItem::create(zero_bias, ID::IMU_CALI_GBIAS).setContext("Calib Gyro", "START").setCallback(button_event_cb);

    /*IMU Info信息页面*/
    auto* imu_info = &EnterableItem::create(parent, ID::IMU_IMU_INFO).setTittle("IMU Info");
    /*IMU类型 只读数据*/
    // LabelItem::create(info,ID::IMU_TYPE).setTittle("IMU Type:");
    StringValShow::create(imu_info, ID::IMU_TYPE)
        .setTittle("IMU Type:")
        .setContext("-")
        .setCallback(string_val_show_update);
    /*IMU状态 只读数据*/
    StringValShow::create(imu_info, ID::IMU_STATE)
        .setTittle("IMU State:")
        .setContext("-")
        .setCallback(string_val_show_update);


    /*MAG Info*/
    auto* mag_info = &EnterableItem::create(parent, ID::IMU_MAG_INFO).setTittle("MAG Info");
    /*MAG类型*/
    StringValShow::create(mag_info, ID::IMU_MAG_TYPE)
        .setTittle("MAG Type:")
        .setContext("-")
        .setCallback(string_val_show_update);
    /*MAG状态*/
    StringValShow::create(mag_info, ID::IMU_MAG_STATE)
        .setTittle("MAG State:")
        .setContext("-")
        .setCallback(string_val_show_update);
}

static void alt_est_info_init(ItemBase* parent)
{
    LabelItem::create(parent, ID::ALT_EST_ALGO).setTittle("Algorithm: AEKF-AE6");
    FloatValShow::create(parent, ID::ALT_EST_R).setContext("matrix-R", "%.0f").setCallback(alt_est_update);
    FloatValShow::create(parent, ID::ALT_EST_P).setContext("matrix-P", "%.1f").setCallback(alt_est_update);
    FloatValShow::create(parent, ID::ALT_EST_CHI).setContext("Chi Square", "%.3f").setCallback(alt_est_update);
    FloatValShow::create(parent, ID::ALT_EST_EK).setContext("acc error", "%.3f").setCallback(alt_est_update);
    FloatValShow::create(parent, ID::ALT_EST_ACC_NORM).setContext("acc norm", "%.3f").setCallback(alt_est_update);
}


static float imu_data_update(uint32_t id)
{
    float tmp = 0;
    if (xSemaphoreTake(ahrs.output.lock, 20) == pdTRUE)
    {
        if (id == ID::IMU_PITCH)
        {
            tmp = ahrs.output.pitch;
        }
        else if (id == ID::IMU_ROLL)
        {
            tmp = ahrs.output.roll;
        }
        else if (id == ID::IMU_YAW)
        {
            tmp = ahrs.output.yaw;
        }
        else if (id == ID::IMU_ACCX)
        {
            tmp = ahrs.output.acc_x;
        }
        else if (id == ID::IMU_ACCY)
        {
            tmp = ahrs.output.acc_y;
        }
        else if (id == ID::IMU_ACCZ)
        {
            tmp = ahrs.output.acc_z;
        }
        else if (id == ID::IMU_GYRX)
        {
            tmp = ahrs.output.gyr_x;
        }
        else if (id == ID::IMU_GYRY)
        {
            tmp = ahrs.output.gyr_y;
        }
        else if (id == ID::IMU_GYRZ)
        {
            tmp = ahrs.output.gyr_z;
        }
        else if (id == ID::IMU_MAGX)
        {
            tmp = ahrs.output.mag_x;
        }
        else if (id == ID::IMU_MAGY)
        {
            tmp = ahrs.output.mag_y;
        }
        else if (id == ID::IMU_MAGZ)
        {
            tmp = ahrs.output.mag_z;
        }
        else if (id == ID::IMU_ICMTEMP)
        {
            tmp = ahrs.output.imu_temp;
        }
        else if (id == ID::IMU_LIS3TEMP)
        {
            tmp = ahrs.output.mag_temp;
        }
        xSemaphoreGive(ahrs.output.lock);
    }
    return tmp;
}

static float imu_bias_update(uint32_t id)
{
    float tmp = 0;
    if (id == ID::IMU_BIAS_AX)
    {
        tmp = ahrs.imu_sbias.acc_x;
    }
    else if (id == ID::IMU_BIAS_AY)
    {
        tmp = ahrs.imu_sbias.acc_y;
    }
    else if (id == ID::IMU_BIAS_AZ)
    {
        tmp = ahrs.imu_sbias.acc_z;
    }
    else if (id == ID::IMU_BIAS_GX)
    {
        tmp = ahrs.imu_sbias.gyr_x;
    }
    else if (id == ID::IMU_BIAS_GY)
    {
        tmp = ahrs.imu_sbias.gyr_y;
    }
    else if (id == ID::IMU_BIAS_GZ)
    {
        tmp = ahrs.imu_sbias.gyr_z;
    }
    return tmp;
}


static int int_val_show_update(uint32_t id)
{
    int tmp = 0;

    return tmp;
}

static float sys_update(uint32_t id)
{
    float tmp = 0;
    if (xSemaphoreTake(car.mutex, 200) == pdTRUE)
    {
        if (id == ID::SYS_SYSCLOCK)
        {
            tmp = (float)car.coreClock / 1e6;
        }
        else if (id == ID::SYS_MCUTEMP)
        {
            tmp = car.mcu_temp;
        }
        else if (id == ID::SYS_VCCVOLT)
        {
            tmp = car.mcu_volt;
        }
        xSemaphoreGive(car.mutex);
    }
    return tmp;
}

static float battary_info_update(uint32_t id)
{
    float tmp = 0;
    if (xSemaphoreTake(car.mutex, 200) == pdTRUE)
    {
        if (id == ID::SYS_BATTARYINFO_VOLTAGE)
        {
            tmp = car.batt_volt;
        }
        else if (id == ID::SYS_BATTARYINFO_CURRENT)
        {
            tmp = car.batt_curr;
        }
        else if (id == ID::SYS_BATTARYINFO_POWER)
        {
            tmp = car.batt_power;
        }
        xSemaphoreGive(car.mutex);
    }
    return tmp;
}

static float alt_est_update(uint32_t id)
{
    float tmp = 0;

    if (xSemaphoreTake(ahrs.ekf_altest6_info.lock, 200) == pdTRUE)
    {
        if (id == ID::ALT_EST_R)
        {
            tmp = ahrs.ekf_altest6_info.trace_R;
        }
        else if (id == ID::ALT_EST_P)
        {
            tmp = ahrs.ekf_altest6_info.trace_P;
        }
        else if (id == ID::ALT_EST_CHI)
        {
            tmp = ahrs.ekf_altest6_info.chi_square;
        }
        else if (id == ID::ALT_EST_EK)
        {
            tmp = ahrs.ekf_altest6_info.trace_acc_err;
        }
        else if (id == ID::ALT_EST_ACC_NORM)
        {
            tmp = ahrs.ekf_altest6_info.acc_norm;
        }
        xSemaphoreGive(ahrs.ekf_altest6_info.lock);
    }
    return tmp;
}

static void button_event_cb(ItemBase* item, uint32_t id)
{
    if (id == ID::BALANCE_CTRL)
    {
        g_ctrl.blc_en = !g_ctrl.blc_en;
    }
    else if (id == ID::SYS_FERAMSETTINGS_CLEAR)
    {
        sAPP_Tasks_StartFormatFeRAM();
    }
    else if (id == ID::IMU_CALI_ABIAS)
    {
        sAPP_Tasks_StartCalibAccBias();
    }
    else if (id == ID::IMU_CALI_GBIAS)
    {
        sAPP_Tasks_StartCalibGyrBias();
    }
}

static void int_val_adj_event_cb(IntValAdj* item, uint32_t id, int value)
{
    if (id == ID::POWER_LIGHT)
    {
        sDRV_PL_SetBrightness((float)value);
    }
}

bool is_frist = true;


const char* string_val_show_update(uint32_t id)
{
    if (is_frist == true)
    {
        is_frist = false;
    }
    if (id == ID::IMU_TYPE)
    {
        if (ahrs.imu_type == AHRS::IMUType::ICM45686)
        {
            return "ICM45686";
        }
        else
        {
            return "Unknown";
        }
    }
    else if (id == ID::IMU_MAG_TYPE)
    {
        if (ahrs.mag_type == AHRS::MAGType::LIS3MDLTR)
        {
            return "LIS3MDL";
        }
        else
        {
            return "Unknown";
        }
    }
    else if (id == ID::IMU_STATE)
    {
        if (ahrs.imu_state == AHRS::IMUState::OK)
        {
            return "OK";
        }
        else if (ahrs.imu_state == AHRS::IMUState::NEED_CALIB)
        {
            return "NEED CALI";
        }
        else
        {
            return "Unknown";
        }
    }
    else if (id == ID::IMU_MAG_STATE)
    {
        if (ahrs.mag_state == AHRS::MAGState::OK)
        {
            return "OK";
        }
        else if (ahrs.mag_state == AHRS::MAGState::NEED_CALIB)
        {
            return "NEED CALI";
        }
        else if (ahrs.mag_state == AHRS::MAGState::NO_MAG)
        {
            return "NO MAG";
        }
        else if (ahrs.mag_state == AHRS::MAGState::DATA_DISTURBED)
        {
            return "DISTURB";
        }
        else
        {
            return "Unknown";
        }
    }
    return "";
}
