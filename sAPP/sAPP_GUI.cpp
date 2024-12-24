#include "sAPP_GUI.hpp"


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

static void imu_menu_init(sLM_TreeNode* parent);
static void feram_menu_init(sLM_TreeNode* parent);
static void pwrlight_menu_init(sLM_TreeNode* parent);



static void imu_data_update(void* param,uint32_t _param_tag){
    if(xSemaphoreTake(ahrs.mutex,20) == pdTRUE){
        if(_param_tag == TAG_IMU_PITCH){
            *(float*)param = ahrs.dat.acc_x;
        }
        else if(_param_tag == TAG_IMU_ROLL){
            *(float*)param = ahrs.dat.roll;
        }
        else if(_param_tag == TAG_IMU_YAW){
            *(float*)param = ahrs.dat.yaw;
        }
        else if(_param_tag == TAG_IMU_ACC_X){
            *(float*)param = ahrs.dat.acc_x;
        }
        else if(_param_tag == TAG_IMU_ACC_Y){
            *(float*)param = ahrs.dat.acc_y;
        }
        else if(_param_tag == TAG_IMU_ACC_Z){
            *(float*)param = ahrs.dat.acc_z;
        }
        else if(_param_tag == TAG_IMU_GYR_X){
            *(float*)param = ahrs.dat.gyr_x;
        }
        else if(_param_tag == TAG_IMU_GYR_Y){
            *(float*)param = ahrs.dat.gyr_y;
        }
        else if(_param_tag == TAG_IMU_GYR_Z){
            *(float*)param = ahrs.dat.gyr_z;
        }


        xSemaphoreGive(ahrs.mutex);
    }
    
}

static sLM::Item_ParamCbRetType button_event_cb(void* param,sLM::Item_ParamType _param_type){
    if(_param_type == sLM::Item_ParamType::BUTTON_PRESS){
        if(*(uint32_t*)param == TAG_CALI_IMU){
            
            sAPP_Tasks_StartCalibrateIMU();
        }
        else if(*(uint32_t*)param == TAG_CLEAR_FERAM){
            sAPP_Tasks_StartFormatFeRAM();
        }
        else if(*(uint32_t*)param == TAG_BLC_CTRL){
            g_ctrl.blc_en = !g_ctrl.blc_en;
        }
    }
    return sLM::Item_ParamCbRetType::UNLOCK;
}

static sLM::Item_ParamCbRetType pwrlight_event_cb(void* param,sLM::Item_ParamType _param_type){
    uint32_t val = *(uint32_t*)param;
    sDRV_PL_SetBrightness((float)val);
    return sLM::Item_ParamCbRetType::UNLOCK;
}



void sAPP_GUI_Init(){
    slm.init();
    slm_weights.init(&oled,&slm);

    imu_menu_init(slm.root);
    feram_menu_init(slm.root);
    


    sLM::MenuItemData data;
    sLM::ItemDataCreateConf conf;
    conf.access = sLM::Item_ParamAccess::RW;
    conf.param_type = sLM::Item_ParamType::INT;
    conf.item_type = sLM::ItemType::BUTTON;
    conf.change_cb = button_event_cb;
    conf.update_cb = nullptr;
    conf.change_method = sLM::Item_ParamCbMethod::NON_CB;
    conf.limit_type = sLM::Item_ParamLimitType::NO;
    conf.param_tag = TAG_CALI_IMU;


    conf.item_type = sLM::ItemType::BUTTON;
    conf.change_cb = button_event_cb;
    conf.param_tag = TAG_BLC_CTRL;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Blc Ctrl","",0,0,0,0,0);
    sLM_TreeNode* ctrl           = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(slm.root,ctrl);

    pwrlight_menu_init(slm.root);


}

static void imu_menu_init(sLM_TreeNode* parent){
    sLM::MenuItemData data;
    sLM::ItemDataCreateConf conf;
    conf.access = sLM::Item_ParamAccess::RW;
    conf.param_type = sLM::Item_ParamType::INT;
    conf.item_type = sLM::ItemType::NORMAL;
    conf.change_cb = sLM::param_change;
    conf.update_cb = nullptr;
    conf.change_method = sLM::Item_ParamCbMethod::NON_CB;
    conf.limit_type = sLM::Item_ParamLimitType::NO;

    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"IMU","",0,0,0,0,0);
    sLM_TreeNode* imu           = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(parent,imu);

    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Data Overview","",0,0,0,0,0);
    sLM_TreeNode* imu_data      = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu,imu_data);

    conf.item_type = sLM::ItemType::BUTTON;
    conf.change_cb = button_event_cb;
    conf.param_tag = TAG_CALI_IMU;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Calibrate IMU","",0,0,0,0,0);
    sLM_TreeNode* cali_imu      = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu,cali_imu);


    conf.item_type = sLM::ItemType::NORMAL;
    conf.param_type = sLM::Item_ParamType::FLOAT;
    conf.access = sLM::Item_ParamAccess::RO;
    conf.param_tag = TAG_IMU_PITCH;
    conf.update_cb = imu_data_update;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Pitch:","",0,0,0,0,0);
    sLM_TreeNode* pitch         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,pitch);

    conf.param_tag = TAG_IMU_ROLL;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Roll:","",0,0,0,0,0);
    sLM_TreeNode* roll         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,roll);

    conf.param_tag = TAG_IMU_YAW;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Yaw:","",0,0,0,0,0);
    sLM_TreeNode* yaw         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,yaw);

    conf.param_tag = TAG_IMU_ACC_X;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Acc-X:","",0,0,0,0,0);
    sLM_TreeNode* acc_x         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,acc_x);

    conf.param_tag = TAG_IMU_ACC_Y;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Acc-Y:","",0,0,0,0,0);
    sLM_TreeNode* acc_y         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,acc_y);

    conf.param_tag = TAG_IMU_ACC_Z;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Acc-Z:","",0,0,0,0,0);
    sLM_TreeNode* acc_z         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,acc_z);

    conf.param_tag = TAG_IMU_GYR_X;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Gyro-X:","",0,0,0,0,0);
    sLM_TreeNode* gyr_x         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,gyr_x);

    conf.param_tag = TAG_IMU_GYR_Y;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Gyro-Y:","",0,0,0,0,0);
    sLM_TreeNode* gyr_y         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,gyr_y);

    conf.param_tag = TAG_IMU_GYR_Z;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Gyro-Z:","",0,0,0,0,0);
    sLM_TreeNode* gyr_z         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,gyr_z);

}

static void feram_menu_init(sLM_TreeNode* parent){
    sLM::MenuItemData data;
    sLM::ItemDataCreateConf conf;
    conf.access = sLM::Item_ParamAccess::RW;
    conf.param_type = sLM::Item_ParamType::INT;
    conf.item_type = sLM::ItemType::NORMAL;
    conf.change_cb = sLM::param_change;
    conf.update_cb = nullptr;
    conf.change_method = sLM::Item_ParamCbMethod::NON_CB;
    conf.limit_type = sLM::Item_ParamLimitType::NO;

    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"FeRAM","",0,0,0,0,0);
    sLM_TreeNode* feram           = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(parent,feram);

    conf.item_type = sLM::ItemType::BUTTON;
    conf.change_cb = button_event_cb;
    conf.param_tag = TAG_CLEAR_FERAM;
    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Clear All to 0","",0,0,0,0,0);
    sLM_TreeNode* clear           = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(feram,clear);



}


static void pwrlight_menu_init(sLM_TreeNode* parent){
    sLM::MenuItemData data;
    sLM::ItemDataCreateConf conf;
    conf.access = sLM::Item_ParamAccess::RW;
    conf.param_type = sLM::Item_ParamType::INT;
    conf.item_type = sLM::ItemType::NORMAL;
    conf.change_cb = pwrlight_event_cb;
    conf.update_cb = nullptr;
    conf.change_method = sLM::Item_ParamCbMethod::CHRG_CB;
    conf.limit_type = sLM::Item_ParamLimitType::RANGE;

    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Pwr Light","%",0,0,100,20,20);
    sLM_TreeNode* pwrlight = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(parent,pwrlight);
}






