#pragma once

#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"

#include <string>
#include <cstring>

#include "sBSP_UART.h"

#include "sLM_Tree.hpp"


//item的显示文本字符串长度
#define SLM_ITEM_TEXT_LEN   16
//item的显示的字符串参数最大的长度(byte),不能小于int的长度(4)
#define SLM_MAX_ISP_STR_LEN 16
//参数的单位字段长度
#define SLM_ITEM_UNIT_LEN   6

#define SLM_ROOT_MENU_TEXT  "sGCARCv5.1"


//little~
class sLittleMenu{
public:
    enum class Item_ParamAccess{    //参数权限
        NO = 0,         //没有参数
        RW,             //参数可读写
        RO,             //参数只读
    };
    enum class Item_ParamType{      //参数类型
        INT = 0,        //显示的是int
        FLOAT,          //float类型
        STRING,         //char[]
        BUTTON_PRESS,   //按键按下事件
    };
    enum class Item_ParamLimitType{ //参数的上下限
        NO = 0,         //无限制
        MIN,            //有最小值
        MAX,            //有最大值
        RANGE,          //范围
    };
    enum class Item_ParamCbMethod{  //参数变化的回调方式
        NON_CB = 0,     //不回调
        CHRG_CB = 1,    //当参数改变时调用回调
        EXIT_CB = 2,    //用户退出选择时调用回调
    };
    enum class Item_ParamCbRetType{ //参数变化回调的返回值
        UNLOCK = 0,     //解锁,允许下一次修改
        LOCK = 1,       //不让用户更改了,等待用户的修改完成后解锁
    };
    enum class Item_ParamChrgDir{   //参数变化的方向,用来通知回调
        UP = 0,
        DN = 1,
    };
    //参数被修改的回调
    using Item_ParamCb = Item_ParamCbRetType(*)(void* param,Item_ParamType type);
    //参数手动更新回调
    using Item_ParamUpdateCb = void(*)(void* param,uint32_t _param_tag);
    //参数上下界保存
    struct Item_Param{
        uint32_t param_tag;                 //参数标签,用于从用户获取数据告诉用户的
        Item_ParamAccess access;            //参数是否可读
        Item_ParamType type;                //参数类型
        Item_ParamLimitType lim_type;       //参数的上下限类型
        Item_ParamCbMethod cb_mthd;         //参数变化的回调方式
        Item_ParamCbRetType status;         //此菜单的锁定状态
        bool is_param_chrg;                 //参数是否变化
        Item_ParamCb cb_func;               //参数变化回调
        Item_ParamUpdateCb update_cb;       //参数手动更新回调
        uint8_t increment[4];               //按下向上操作的增量
        uint8_t decrement[4];               //按下向下操作的增量
        uint8_t max[4];                     //向上的上界
        uint8_t min[4];                     //向下的下界
        uint8_t value[SLM_MAX_ISP_STR_LEN]; //储存显示的参数
        uint8_t unit[SLM_ITEM_UNIT_LEN];    //单位字段
    };
    enum class MenuChildShowMode{           //当前节点的子节点显示方式
        LIST = 0,   //目前只实现了list
        PAGE,
    };
    enum class ItemType{                    //item类型
        NORMAL = 0, //显示参数/下一级/调整参数
        BUTTON = 1, //是一个按钮,只能按,无参数
        SWITCH = 2, //一个开关,两种状态,开和关
    };
    struct MenuItemData{                    //item的数据结构体
        uint32_t id;                        //唯一id,手动分配
        char text[SLM_ITEM_TEXT_LEN];       //显示的文本
        Item_Param param;                   //参数部分
        bool is_selected;                   //是否选中
        bool is_hover;                      //光标是否悬停在上面
        MenuChildShowMode child_show_mode;  //子菜单显示模式
        ItemType type;                      //item的类型
        bool switch_on;                     //开关是否为on
    };
    struct ItemDataCreateConf{              //用于创建item data的配置
        uint32_t param_tag;
        Item_ParamAccess access;
        Item_ParamType param_type;
        ItemType item_type;
        Item_ParamCb change_cb;
        Item_ParamUpdateCb update_cb;
        Item_ParamCbMethod change_method;
        Item_ParamLimitType limit_type;
        char text[SLM_ITEM_TEXT_LEN];
    };
    enum class OpEvent{                     //操作事件类型
        NONE = 0,
        PREV,
        NEXT,
        ENTER,
        BACK,
    };
    
    uint32_t id_count;       //id计数器

    sLM_TreeNode* root;      //根节点
    sLM_TreeNode* curr_page; //当前菜单位置

    sLittleMenu();
    ~sLittleMenu();

    void init();
    //添加一个子节点
    int addSubMenu(sLM_TreeNode* parent, sLM_TreeNode* child);
    //设置item数据
    static void setItemData(MenuItemData* item_data,ItemDataCreateConf* config);
    //获取当前节点数据
    MenuItemData& getNodeData();
    //获取任意节点数据
    static MenuItemData& getNodeData(sLM_TreeNode* node);
    //获取同级的第一个节点
    sLM_TreeNode& getFristNode();
    //使用任意兄弟节点获取同级的第一个节点
    sLM_TreeNode& getFristNode(sLM_TreeNode* any_sibling);
    //获取当前菜单项的同级的索引,从1开始
    uint32_t getCurrMenuIndex();
    //获取当前菜单项同级有多少个菜单
    uint32_t getCurrMenuNumber();
    //返回当前同级的第多少个菜单项
    sLM_TreeNode* getIndexMenu(uint32_t index);
    //返回任意兄弟菜单的索引
    sLM_TreeNode* getIndexMenu(sLM_TreeNode* any_slibing ,uint32_t index);
    void reset();
    //参数增量了处理
    void incDecParamHandler(MenuItemData& item,Item_ParamChrgDir dir);
    //设置item数据,int
    static void setItemData(MenuItemData* item_data,const char* _text,const char* _unit,\
                            int _value,int _min,int _max,int _inc,int _dec);
    //设置item数据,float
    static void setItemData(MenuItemData* item_data,const char* _text,const char* _unit,\
                            float _value,float _min,float _max,float _inc,float _dec);

    /*菜单操作,操作有效返回true*/
    void opEnter();
    void opBack();
    void opPrev();
    void opNext();

    void changeHandler(OpEvent ev);

    void update();

    static Item_ParamCbRetType param_change(void* param,Item_ParamType type);

private:
    
};

//littleMenu名字太长了,起个alias~
using sLM = sLittleMenu;


extern sLittleMenu slm;


void printMenuItemData(unsigned char* data, size_t data_size);

