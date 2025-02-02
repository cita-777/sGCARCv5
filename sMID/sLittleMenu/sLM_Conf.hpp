#pragma once

#include <cstring>
#include <string>
#include "stdint.h"
#include "stdbool.h"




//调试日志开关
// #define SLM_DEBUG_LOG_EN

/*内存分配方式*/
//使用标准库
//#define MEM_ALLOCATOR_STDLIB
//使用FreeRTOS
#define MEM_ALLOCATOR_FREERTOS


//item的显示文本字符串长度
#define SLM_ITEM_TEXT_LEN           16
//item的显示的字符串参数最大的长度(byte)
#define SLM_MAX_PARAM_STR_LEN       8
//参数的单位字段长度
#define SLM_ITEM_UNIT_LEN           6
//根节点的标题文字
#define SLM_ROOT_MENU_TEXT          "sGCARCv5.2"
//锁定时提示框标题长度
#define SLM_LOCK_TITTLE_LEN         16
//锁定时提示框信息长度
#define SLM_LOCK_MESSAGE_LEN        32
//默认锁定时提示框标题
#define SLM_LOCK_DEFAULT_TITTLE     "sLittleMenu"
//默认锁定时提示框信息
#define SLM_LOCK_DEFAULT_MESSAGE    "Please wait a \nmoment..."

//显示格式 eg:5%
#define SLM_LIST_PARAM_SHOW_FMT_LEN 12
#define SLM_LIST_PARAM_SHOW_INT     " %d%s"
#define SLM_LIST_PARAM_SHOW_FLOAT   " %.2f%s"

//最大菜单项数量
#define SLM_MAX_ITEM_NUM            128

#define SLM_INT_VAL_ADJ_DEFAULT     "%d"
#define SLM_INT_VAL_ADJ_VAL_LEN     12

#define SLM_FLOAT_VAL_ADJ_DEFAULT   "%.1f"
#define SLM_FLOAT_VAL_ADJ_VAL_LEN   12

#define SLM_SWITCH_TEXT_LEN          6
#define SLM_SWITCH_ON_TEXT_DEFAULT  "ON"
#define SLM_SWITCH_OFF_TEXT_DEFAULT "OFF"

#define SLM_INT_VAL_SHOW_DEFAULT    "%d"
#define SLM_INT_VAL_SHOW_VAL_LEN    12

#define SLM_FLOAT_VAL_SHOW_DEFAULT  "%.1f"
#define SLM_FLOAT_VAL_SHOW_VAL_LEN  12



#ifdef SLM_DEBUG_LOG_EN
    #include "sDBG_Debug.h"
#endif



#ifdef MEM_ALLOCATOR_STDLIB
    #include <stdlib.h>
    #include <new>
    #define SLM_PORT_MALLOC         malloc
    #define SLM_PORT_FREE           free
#endif

#ifdef MEM_ALLOCATOR_FREERTOS
    #include "FreeRTOS.h"
    // #include "task.h"
    #include <new>
    #define SLM_PORT_MALLOC         pvPortMalloc
    #define SLM_PORT_FREE           vPortFree
#endif


#ifdef SLM_DEBUG_LOG_EN
    #define SLM_LOG_INFO(_TEXT) dbg_info("sLM line:%u,%s\n",__LINE__,_TEXT)
    #define SLM_LOG_WARN(_TEXT) dbg_warn("sLM line:%u,%s\n",__LINE__,_TEXT)
    #define SLM_LOG_ERR(_TEXT)  dbg_printf("[ERR ] sLM line:%u,%s\n",__LINE__,_TEXT)
    #define SLM_PRINTF          dbg_printf
#else
    #define SLM_LOG_INFO(_TEXT) do{(void)0;}while(0)
    #define SLM_LOG_WARN(_TEXT) do{(void)0;}while(0)
    #define SLM_LOG_ERR(_TEXT)  do{(void)0;}while(0)
    #define SLM_PRINTF(...)     do{(void)0;}while(0)
#endif

//placement new创建类
#define SLM_CREATE_CLASS(_ITEM_NAME,_PARAM,...) new(SLM_PORT_MALLOC(sizeof(_ITEM_NAME))) _ITEM_NAME(_PARAM);






