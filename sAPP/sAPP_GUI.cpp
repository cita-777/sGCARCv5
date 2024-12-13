#include "sAPP_GUI.hpp"




static void imu_menu_init(sLM_TreeNode* parent);


#define TAG_IMU_PITCH 1
#define TAG_IMU_ROLL  2
#define TAG_IMU_YAW   3
#define TAG_IMU_ACC_X 4
#define TAG_IMU_ACC_Y 5
#define TAG_IMU_ACC_Z 6
#define TAG_IMU_GYR_X 7
#define TAG_IMU_GYR_Y 8
#define TAG_IMU_GYR_Z 9
#define TAG_IMU_MAG_X 10
#define TAG_IMU_MAG_Y 11
#define TAG_IMU_MAG_Z 12



static void imu_data_update(void* param,uint32_t _param_tag){
    if(xSemaphoreTake(ahrs.get_data_mutex,20) == pdTRUE){
        if(_param_tag == TAG_IMU_PITCH){
            *(float*)param = ahrs.pitch;
        }
        else if(_param_tag == TAG_IMU_ROLL){
            *(float*)param = ahrs.roll;
        }
        else if(_param_tag == TAG_IMU_YAW){
            *(float*)param = ahrs.yaw;
        }
        xSemaphoreGive(ahrs.get_data_mutex);
    }
    
}

void sAPP_GUI_Init(){
    slm.init();
    slm_weights.init(&oled,&slm);

    imu_menu_init(slm.root);
}

static void imu_menu_init(sLM_TreeNode* parent){
    sLM::MenuItemData data;
    sLM::ItemDataCreateConf conf;
    conf.access = sLM::Item_ParamAccess::RW;
    conf.type = sLM::Item_ParamType::INT;
    conf.change_cb = sLM::param_change;
    conf.update_cb = nullptr;
    conf.change_method = sLM::Item_ParamCbMethod::NON_CB;
    conf.limit_type = sLM::Item_ParamLimitType::NO;

    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"IMU Data","",0,0,0,0,0);
    sLM_TreeNode* imu_data      = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(parent,imu_data);

    conf.type = sLM::Item_ParamType::FLOAT;
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

    conf.access = sLM::Item_ParamAccess::RW;
    conf.type = sLM::Item_ParamType::INT;
    conf.change_cb = sLM::param_change;
    conf.update_cb = nullptr;
    conf.change_method = sLM::Item_ParamCbMethod::NON_CB;
    conf.limit_type = sLM::Item_ParamLimitType::NO;

    sLM::setItemData(&data,&conf);
    sLM::setItemData(&data,"Clear FeRAM","",0,0,0,0,0);
    //sLM::setItemParamUpdateCb(&data,gyr_z_update);
    sLM_TreeNode* clear         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(slm.root,clear);

    //todo 实现一个按钮button,不是Switch




    
    

}




