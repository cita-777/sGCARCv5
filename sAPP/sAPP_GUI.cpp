#include "sAPP_GUI.hpp"





static void acc_x_update(void* param){
    *(float*)param = ahrs.acc_x;
}

static void acc_y_update(void* param){
    *(float*)param = ahrs.acc_y;
}

static void acc_z_update(void* param){
    *(float*)param = ahrs.acc_z;
}

static void gyr_x_update(void* param){
    *(float*)param = ahrs.gyr_x;
}

static void gyr_y_update(void* param){
    *(float*)param = ahrs.gyr_y;
}

static void gyr_z_update(void* param){
    *(float*)param = ahrs.gyr_z;
}

static void pitch_update(void* param){
    *(float*)param = ahrs.pitch;
}

static void roll_update(void* param){
    *(float*)param = ahrs.roll;
}

static void yaw_update(void* param){
    *(float*)param = ahrs.yaw;
}


void sAPP_GUI_Init(){
    slm.init();

    sLM::MenuItemData data;
    sLM::ItemDataCreateConf conf;
    conf.access = sLM::Item_ParamAccess::RW;
    conf.type = sLM::Item_ParamType::INT;
    conf.change_cb = sLM::param_change;
    conf.change_method = sLM::Item_ParamCbMethod::NON_CB;
    conf.limit_type = sLM::Item_ParamLimitType::NO;

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"IMU Data","",0,0,0,0,0);
    sLM_TreeNode* imu_data      = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(slm.root,imu_data);

    conf.type = sLM::Item_ParamType::FLOAT;
    conf.access = sLM::Item_ParamAccess::RO;

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Pitch:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,pitch_update);
    sLM_TreeNode* pitch         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,pitch);

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Roll:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,roll_update);
    sLM_TreeNode* roll         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,roll);

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Yaw:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,yaw_update);
    sLM_TreeNode* yaw         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,yaw);

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Acc-X:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,acc_x_update);
    sLM_TreeNode* acc_x         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,acc_x);

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Acc-Y:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,acc_y_update);
    sLM_TreeNode* acc_y         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,acc_y);

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Acc-Z:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,acc_z_update);
    sLM_TreeNode* acc_z         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,acc_z);

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Gyro-X:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,gyr_x_update);
    sLM_TreeNode* gyr_x         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,gyr_x);

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Gyro-Y:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,gyr_y_update);
    sLM_TreeNode* gyr_y         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,gyr_y);

    sLM::createItemData(&data,&conf);
    sLM::setItemData(&data,"Gyro-Z:","",0,0,0,0,0);
    sLM::setItemParamUpdateCb(&data,gyr_z_update);
    sLM_TreeNode* gyr_z         = sLM_TreeNode::createNode((const void*)&data,sizeof(data));
    slm.addSubMenu(imu_data,gyr_z);




    
    slm_weights.init(&oled,&slm);;

}




