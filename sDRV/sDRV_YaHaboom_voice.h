#ifndef SDRV_YAHABOOM_VOICE_H
#define SDRV_YAHABOOM_VOICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------include-----------------------------------*/
#include "sBSP_I2C.h"
#include "stm32f4xx_hal.h"

/*-----------------------------------macro------------------------------------*/
#define VOICE_ADDR 0x2B       // 语音模块I2C地址
#define WRITE_REGISTER 0x03   // 写寄存器命令
#define READ_REGISTER 0x64    // 读寄存器命令

// 主动播报内容定义
#define VOICE_START_STRAIGHT_WALK 0x8D      // 开启直线行走模式
#define VOICE_START_DIRECTION_REPORT 0x8E   // 开启定时播报当前方向
#define VOICE_LOW_BATTERY_WARNING 0x8F      // 当前电量不足,请及时充电
#define VOICE_QUERY_BATTERY_STATUS 0x91     // 现在电量状况
#define VOICE_BATTERY_SUFFICIENT 0x92       // 当前电量充足
#define VOICE_FALL_DETECTED_ALARM 0x93      // 检测到跌落,开启报警
#define VOICE_OBSTACLE_30CM_WARNING 0x94    // 前方障碍物不足三十厘米,注意安全
#define VOICE_OBSTACLE_30CM_WARNING 0x94    // 前方障碍物不足三十厘米,注意安全
#define VOICE_OBSTACLE_10CM_ALARM 0x95      // 前方障碍物不足十厘米,开启报警
#define VOICE_SELF_CHECK_PASS 0x96          // 自检通过,智能导盲助手已启动
#define VOICE_SELF_CHECK_FAIL 0x97          // 自检失败,请进行维修
#define VOICE_FACING_SOUTH 0x98             // 目前朝向南方
#define VOICE_FACING_NORTH 0x99             // 目前朝向北方
#define VOICE_FACING_EAST 0x9A              // 目前朝向东方
#define VOICE_FACING_WEST 0x9B              // 目前朝向西方
#define VOICE_STOP_DIRECTION_REPORT 0x9C    // 关闭定时播报当前方向
#define VOICE_INIT 0x67                     // 初始化语音

/*----------------------------------typedef-----------------------------------*/
// 语音识别结果枚举
typedef enum
{
    VOICE_RESULT_NONE = 0,
    VOICE_RESULT_START_STRAIGHT_WALK,
    VOICE_RESULT_START_DIRECTION_REPORT,
    VOICE_RESULT_LOW_BATTERY_WARNING,
    VOICE_RESULT_QUERY_BATTERY_STATUS,
    VOICE_RESULT_BATTERY_SUFFICIENT,
    VOICE_RESULT_FALL_DETECTED_ALARM,
    VOICE_RESULT_OBSTACLE_30CM_WARNING,
    VOICE_RESULT_OBSTACLE_10CM_ALARM,
    VOICE_RESULT_SELF_CHECK_PASS,
    VOICE_RESULT_SELF_CHECK_FAIL,
    VOICE_RESULT_FACING_SOUTH,
    VOICE_RESULT_FACING_NORTH,
    VOICE_RESULT_FACING_EAST,
    VOICE_RESULT_FACING_WEST,
    VOICE_RESULT_STOP_DIRECTION_REPORT,
    VOICE_RESULT_INIT
} voice_result_t;

/*----------------------------------variable----------------------------------*/

/*-------------------------------------os-------------------------------------*/

/*----------------------------------function----------------------------------*/
// 语音模块初始化
int8_t sDRV_YaHaboom_Voice_Init(void);

// 播报指定内容
int8_t sDRV_YaHaboom_Voice_Broadcast(uint8_t data);

// 读取语音识别数据
voice_result_t sDRV_YaHaboom_Voice_ReadData(void);

// 语音识别任务处理函数
void sDRV_YaHaboom_Voice_Handler(void);

// 新增的语音播报函数
void               sDRV_YaHaboom_Voice_BroadcastStraightWalk(void);
void               sDRV_YaHaboom_Voice_BroadcastDirectionReport(bool enable);
void               sDRV_YaHaboom_Voice_BroadcastBatteryStatus(bool is_low);
void               sDRV_YaHaboom_Voice_BroadcastObstacleWarning(uint8_t distance_cm);
void               sDRV_YaHaboom_Voice_BroadcastSelfCheck(bool pass);
void               sDRV_YaHaboom_Voice_BroadcastDirection(uint8_t direction);   // 0:南 1:北 2:东 3:西
static const char* get_voice_code_description(uint8_t code);
/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* SDRV_YAHABOOM_VOICE_H */
