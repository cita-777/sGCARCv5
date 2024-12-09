#pragma once



#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"

#include <string>
#include <cstring> // 用于 strncpy
#include <cstdlib> // for malloc, free

#include "sBSP_UART.h"


#include "sDRV_GenOLED.h"
#include "sG2D.hpp"


#include "FreeRTOS.h"
#include "task.h"

#include "sLM_Tree.hpp"
#include "sLM_Show.hpp"


#define SLM_ITEM_TEXT_LEN 16
//item的显示的字符串参数最大的长度(byte),不能小于int的长度(4)
#define SLM_MAX_ISP_STR_LEN 8








class sLittleMenu{
public:
    //参数属性
    enum class Item_ParamAccess{
        NO_PARAM = 0,   //没有参数
        RW_PARAM,       //参数可读写
        RO_PARAM,       //参数只读
    };
    //参数类型
    enum class Item_ParamType{
        INT = 0,        //显示的是int
        FLOAT,          //float类型
        STRING,         //char[]
    };
    //参数的上下限
    enum class Item_ParamLimitType{
        NO_LIMIT = 0,   //无限制
        MIN_LIMIT,      //有最小值
        MAX_LIMIT,      //有最大值
        RANGE,          //范围
    };
    //参数变化的回调方式
    enum class Item_ParamCbMethod{
        NON_CB = 0,     //不回调
        CHRG_CB = 1,    //当参数改变时调用回调
        EXIT_CB = 2,    //用户退出选择时调用回调
    };
    enum class Item_ParamCbRetType{
        OK = 0,         //无操作
        LOCK = 1,       //不让用户更改了,等待用户的修改完成后解锁
        UNLOCK = 2,     //解锁,允许下一次修改
    };
    //参数变化的方向,用来通知回调
    enum class Item_ParamChrgDir{
        UP = 0,
        DN = 1,
    };
    //参数被修改的回调
    using Item_ParamCb = Item_ParamCbRetType(*)(void* param,Item_ParamType type,Item_ParamChrgDir dir);

    //参数上下界保存
    struct Item_Param{
        Item_ParamAccess access;
        Item_ParamType type;
        Item_ParamLimitType lim_type;
        Item_ParamCbMethod cb_mthd;
        Item_ParamCb cb;
        //按下向上操作的增量
        union{int i_increment;float f_increment;};
        //按下向下操作的增量
        union{int i_decrement;float f_decrement;};
        //向上的上界
        union{int i_up_val;float f_up_val;};
        //向下的下界
        union{int i_dn_val;float f_dn_val;};
        //储存显示的参数
        union{int i_val;float f_val;char str_val[SLM_MAX_ISP_STR_LEN];};
    };
    //当前节点的子节点显示方式
    enum class MenuChildShowMode{
        LIST = 0,
        PAGE,
    };


    //item的数据结构体
    struct MenuItemData{
        uint32_t id;    //唯一id,手动分配
        char text[SLM_ITEM_TEXT_LEN];    //显示的文本
        Item_Param param;   //参数部分
        bool is_selected;   //是否选中
        bool is_hover;      //光标是否悬停在上面
        MenuChildShowMode child_show_mode;
    };




    enum class OpEvent{
        NONE = 0,
        PREV,
        NEXT,
        ENTER,
        BACK,
    };


    sLM_TreeNode* root;    //根节点
    sLM_TreeNode* curr_page; //当前菜单位置

    sLittleMenu();
    ~sLittleMenu();

    void init();
    int addSubMenu(sLM_TreeNode* parent, sLM_TreeNode* child);
    sLM_TreeNode* getCurrMenu();
    void resetToRoot();

    //参数增加处理,包含限幅
    void increaseParam(MenuItemData& item);
    //参数减少处理,包含限幅
    void decreaseParam(MenuItemData& item);
    MenuItemData& getNodeData();
    static MenuItemData& getNodeData(sLM_TreeNode* node);
    sLM_TreeNode& getFristNode();

    void opEnter();
    void opBack();
    void opPrev();
    void opNext();



    void changeHandler(OpEvent ev);

    void update();



    static Item_ParamCbRetType param_change(void* param,Item_ParamType type,Item_ParamChrgDir dir);

private:
    
};


using sLM = sLittleMenu;



class sLM_Event{

};




void printMenuItemData(const unsigned char* data, size_t data_size);

