/**
 * sLittleMenu.cpp
 * Sightseer's LittleMenu 一个轻量级的菜单~
 * 旨在实现在调参场景快速地进行菜单编写
 * 实现了一个轻量级的菜单,此文件只负责菜单的结构管理,不负责显示,以此实现高通用性
 * 
 * v1.0 241211Night BySightseer.inHNIP9607
 * 第一版完工! demo做好了
 * 下一版改进:
 * todo 完成按钮bool类型
 * 
 * v1.1 
 * 重构代码,使用namespace优化
 * 
 * v1.2 241227 bySightseer.
 * 使用构建者模式重写了菜单item创建过程
 * 
 * v1.3 241229 bySightseer.
 * 加入了耗时操作锁定机制 在锁定时会显示一个对话框 但是机制还不太完善
 * 
 * 
 * 
 * 
 * 
 * 
 */

#pragma once

#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"

#include <string>
#include <cstring>

//用于动态分配内存
#include "FreeRTOS.h"
#include "task.h"


#include "sBSP_UART.h"

#include "sG2D.hpp"

#include "sLM_Renderer.hpp"


//item的显示文本字符串长度
#define SLM_ITEM_TEXT_LEN 16
//item的显示的字符串参数最大的长度(byte)
#define SLM_MAX_PARAM_STR_LEN 8
//参数的单位字段长度
#define SLM_ITEM_UNIT_LEN 6
//根节点的文字
#define SLM_ROOT_MENU_TEXT "sGCARCv5.2"

#define weights_LIST_PARAM_SHOW_POS     80
//显示格式 eg:5%
#define SLM_LIST_PARAM_SHOW_FMT_LEN     12
#define weights_LIST_PARAM_SHOW_INT     " %d%s"
#define weights_LIST_PARAM_SHOW_FLOAT   " %.2f%s"




namespace sLM{



//参数权限
enum class ParamAccess{
    NO = 0,         //没有参数
    RW,             //参数可读写
    RO,             //参数只读
};
//参数类型
enum class ParamType{
    INT = 0,        //显示的是int
    FLOAT,          //float类型
    STRING,         //char[]
    BUTTON_PRESS,   //按键按下事件
};
//参数的上下限
enum class ParamLimitType{
    NO = 0,         //无限制
    MIN,            //有最小值
    MAX,            //有最大值
    RANGE,          //范围
};
//参数被修改的回调方式
enum class ParamModifyCbMethod{
    NON_CB = 0,     //不回调
    CHRG_CB = 1,    //当参数改变时调用回调
    EXIT_CB = 2,    //用户退出选择时调用回调
};
//参数变化回调的返回值
enum class ParamModifyLock{
    UNLOCK = 0,     //解锁,允许下一次修改
    LOCK = 1,       //不让用户更改了,等待用户的修改完成后解锁
};
//参数变化的方向,用来通知回调
enum class ParamModifyDir{
    UP = 0,
    DN = 1,
};
//参数被修改通知用户的回调,形参:指向被修改了的参数的指针,参数的类型,
//如果item是一个button,那么当button按下时,也会调用这个,但是指向参数的指针无效
//如果item是一个Switch,那么当Switch拨到on的时候,参数是int对应1,off对应int 0
using ParamModifyCb = ParamModifyLock(*)(void* param,ParamType _type);
//参数手动更新回调,需要显示时,会先自动调用这个函数获取最新的参数,然后再显示出来
//形参:指向参数的指针,参数tag(注册item时注册的,用于告诉用户是要更新哪个参数)
using ParamManualUpdateCb = void(*)(void* param,uint32_t _tag);

//参数
struct Param{
    uint32_t param_tag;                 //参数标签,用于从用户获取数据时告诉用户的
    ParamAccess access;                 //参数访问权限
    ParamType type;                     //参数类型
    ParamLimitType lim_type;            //参数的上下限类型
    ParamModifyCbMethod modify_cb_mthd; //参数变化的回调方式
    ParamModifyLock lock;               //此菜单的锁定状态
    bool is_param_chrg;                 //参数是否变化了
    ParamModifyCb modify_cb;            //参数变化回调
    ParamManualUpdateCb update_cb;      //参数手动更新回调
    union{int inc_i;float inc_f;};      //按下向上操作的增量
    union{int dec_i;float dec_f;};      //按下向下操作的增量
    union{int max_i;float max_f;};      //向上的上界
    union{int min_i;float min_f;};      //向下的下界
    union{                              //参数值
        int val_i;
        float val_f;
        char val_s[SLM_MAX_PARAM_STR_LEN];
    };
    char unit[SLM_ITEM_UNIT_LEN];       //单位字段
    char show_fmt[16];                  //参数的显示格式

    Param():param_tag(0), access(ParamAccess::NO), type(ParamType::INT),
        lim_type(ParamLimitType::NO), modify_cb_mthd(ParamModifyCbMethod::NON_CB),
        lock(ParamModifyLock::UNLOCK), is_param_chrg(false),
        modify_cb(nullptr), update_cb(nullptr),
        inc_i(0), dec_i(0), max_i(0), min_i(0),
        val_i(0) {
        unit[0] = '\0';
        show_fmt[0] = '\0';
    }
};

//当前节点的子节点显示方式
enum class MenuChildShowMode{
    LIST = 0,   //目前只实现了list
    PAGE,
    CANVAS,     //自定义绘图页面
};
//item类型
enum class ItemType{
    NORMAL = 0, //显示参数/下一级/调整参数
    BUTTON = 1, //是一个按钮,只能按,无参数
    SWITCH = 2, //一个开关,两种状态,开和关
};
//item的数据结构体
struct ItemData{
    uint32_t id;                        //唯一id,手动分配
    char text[SLM_ITEM_TEXT_LEN];       //显示的文本
    Param param;                        //参数部分
    bool is_selected;                   //是否选中
    bool is_hover;                      //光标是否悬停在上面
    MenuChildShowMode child_show_mode;  //子菜单显示模式
    ItemType type;                      //item的类型
};

//用于创建item data的配置
struct ItemDataCreateItemConf{
    uint32_t param_tag;
    ParamAccess access;
    ParamType param_type;
    ItemType item_type;
    ParamModifyCb change_cb;
    ParamManualUpdateCb update_cb;
    ParamModifyCbMethod change_method;
    ParamLimitType limit_type;
    char text[SLM_ITEM_TEXT_LEN];
};

//操作事件类型
enum class OpEvent{
    NONE = 0,
    PREV,
    NEXT,
    ENTER,
    BACK,
    LOCK,
};


/**
 * 基础树节点类
 * Sightseer's Little Menu Tree
 * 适用于菜单的普通树的通用数据结构,基于柔性数组实现节点携带的内容任意改变
 * 
 * v1.0 241209 BySightseer.inHNIP9607
 * 实现了一个抽象的普通树数据结构,给sLittleMenu继承用
 */
class TreeNode{
protected:
    //空实现
    TreeNode(){}
    //构造函数设为protected 防止直接new 需要使用createNode分配内存
    TreeNode(size_t dsize) : \
    parent(nullptr),child(nullptr),prev_sibling(nullptr),next_sibling(nullptr),data_size(dsize){}

public:
    TreeNode* parent;       //指向父级节点
    TreeNode* child;        //指向当前节点的子节点
    TreeNode* prev_sibling; //上一个兄弟
    TreeNode* next_sibling; //指向同级的下一个兄弟节点
    size_t data_size;       //这个节点携带的数据节点的大小
    uint8_t data[1];        //柔性数组 实际分配时可变长度

    TreeNode(const TreeNode&) = delete;            //禁用复制
    TreeNode& operator=(const TreeNode&) = delete; //禁用移动
    virtual ~TreeNode();                           //虚析构

    //创建一个节点
    static TreeNode* createNode(const void* src_data, size_t dsize);
    //添加子节点
    int addChild(TreeNode* newChild);

    //打印树,需用户提供一个打印函数用于打印data内容
    void printTree(int level,void (*printFunc)(unsigned char*, size_t));
};

//little~
class sLittleMenu{
public:
    uint32_t id_count;   //id计数器,确保ID唯一

    sLittleMenu();
    ~sLittleMenu();

    void init(Renderer* _renderer);
    //获取根节点
    TreeNode* getRoot();
    //添加一个子节点
    int addSubMenu(TreeNode* parent,TreeNode* child);
    //设置item数据
    static void setItemData(ItemData* item_data,ItemDataCreateItemConf* config);
    //获取当前菜单节点
    sLM::TreeNode* getCurr();
    //获取当前节点数据
    ItemData& getCurrNodeData();
    //获取任意节点数据
    static ItemData& getNodeData(TreeNode* node);
    //获取同级的第一个节点
    TreeNode& getFristNode();
    //使用任意兄弟节点获取同级的第一个节点
    TreeNode& getFristNode(TreeNode* any_sibling);
    //获取当前菜单项的同级的索引,从1开始
    uint32_t getCurrMenuIndex();
    //获取当前菜单项同级有多少个菜单
    uint32_t getCurrMenuNumber();
    //返回当前同级的第多少个菜单项
    TreeNode* getIndexMenu(uint32_t index);
    //返回任意兄弟菜单的索引
    TreeNode* getIndexMenu(TreeNode* any_slibing,uint32_t index);
    void reset();
    //参数增量了处理
    void incDecParamHandler(ItemData& item,ParamModifyDir dir);

    void setLockCurrItem(bool is_lock);


    /*菜单操作*/
    void opEnter();
    void opBack();
    void opPrev();
    void opNext();

    void changeHandler(OpEvent ev);

    void update();


private:
    TreeNode* root;      //根节点
    TreeNode* curr; //当前菜单位置

    Renderer* renderer; //渲染器
};









class CreateItem{
public:


    CreateItem(){
        // 设置默认值
        conf.param_tag = 0;
        conf.access = ParamAccess::NO;
        conf.param_type = ParamType::INT;
        conf.item_type = ItemType::NORMAL;
        conf.change_cb = nullptr;
        conf.update_cb = nullptr;
        conf.change_method = ParamModifyCbMethod::NON_CB;
        conf.limit_type = ParamLimitType::NO;
        memset(conf.text, 0, SLM_ITEM_TEXT_LEN);

        
    }


    CreateItem& setParamTag(uint32_t tag) {
        conf.param_tag = tag;
        param.param_tag = tag;
        return *this;
    }

    CreateItem& setAccess(ParamAccess access) {
        conf.access = access;
        param.access = access;
        return *this;
    }

    CreateItem& setParamType(ParamType type) {
        conf.param_type = type;
        param.type = type;
        return *this;
    }

    CreateItem& setItemType(ItemType type) {
        conf.item_type = type;
        return *this;
    }

    CreateItem& setChangeCallback(ParamModifyCb cb) {
        conf.change_cb = cb;
        param.modify_cb = cb;
        return *this;
    }

    CreateItem& setUpdateCallback(ParamManualUpdateCb cb) {
        conf.update_cb = cb;
        param.update_cb = cb;
        return *this;
    }

    CreateItem& setChangeMethod(ParamModifyCbMethod method) {
        conf.change_method = method;
        param.modify_cb_mthd = method;
        return *this;
    }

    CreateItem& setLimitType(ParamLimitType lim_type) {
        conf.limit_type = lim_type;
        param.lim_type = lim_type;
        return *this;
    }

    CreateItem& setText(const char* text) {
        std::strncpy(conf.text, text, SLM_ITEM_TEXT_LEN - 1);
        conf.text[SLM_ITEM_TEXT_LEN - 1] = '\0';
        std::strncpy(param.unit, "", SLM_ITEM_UNIT_LEN - 1); // 初始化单位
        return *this;
    }

    // 设置整数类型的参数值
    CreateItem& setValue(int value) {
        param.val_i = value;
        return *this;
    }

    // 设置浮点类型的参数值
    CreateItem& setValue(float value) {
        param.val_f = value;
        return *this;
    }

    // 设置字符串类型的参数值
    CreateItem& setValue(const char* value) {
        std::strncpy(param.val_s, value, SLM_MAX_PARAM_STR_LEN - 1);
        param.val_s[SLM_MAX_PARAM_STR_LEN - 1] = '\0';
        return *this;
    }

    // 设置增量（整数）
    CreateItem& setIncrement(int inc) {
        param.inc_i = inc;
        return *this;
    }

    // 设置增量（浮点）
    CreateItem& setIncrement(float inc) {
        param.inc_f = inc;
        return *this;
    }

    // 设置减量（整数）
    CreateItem& setDecrement(int dec) {
        param.dec_i = dec;
        return *this;
    }

    // 设置减量（浮点）
    CreateItem& setDecrement(float dec) {
        param.dec_f = dec;
        return *this;
    }

    // 设置最大值（整数）
    CreateItem& setMax(int max) {
        param.max_i = max;
        return *this;
    }

    // 设置最大值（浮点）
    CreateItem& setMax(float max) {
        param.max_f = max;
        return *this;
    }

    // 设置最小值（整数）
    CreateItem& setMin(int min) {
        param.min_i = min;
        return *this;
    }

    // 设置最小值（浮点）
    CreateItem& setMin(float min) {
        param.min_f = min;
        return *this;
    }

    CreateItem& setIntParamShowFmt(const char* _show_fmt){
        std::strncpy(param.show_fmt,_show_fmt,SLM_LIST_PARAM_SHOW_FMT_LEN - 1);
        return *this;
    }

    // 设置单位
    CreateItem& setUnit(const char* unit) {
        std::strncpy(param.unit, unit, SLM_ITEM_UNIT_LEN - 1);
        param.unit[SLM_ITEM_UNIT_LEN - 1] = '\0';
        return *this;
    }

    // 构建 ItemData 对象
    TreeNode* create() {
        ItemData item;
        item.id = 0; // 可以通过其他方式设置唯一ID
        std::strncpy(item.text, conf.text, SLM_ITEM_TEXT_LEN - 1);
        item.text[SLM_ITEM_TEXT_LEN - 1] = '\0';
        item.param = param;
        item.is_selected = false;
        item.is_hover = false;
        item.child_show_mode = MenuChildShowMode::LIST;
        item.type = conf.item_type;
        std::strncpy(item.param.unit, param.unit, SLM_ITEM_UNIT_LEN - 1);
        item.param.unit[SLM_ITEM_UNIT_LEN - 1] = '\0';
        
        //如果用户没有显式指定显示格式,则帮他指定
        if(item.param.show_fmt[0] == '\0'){
            if(item.param.type == ParamType::INT){
                strcpy(item.param.show_fmt,weights_LIST_PARAM_SHOW_INT);
            }
            else if(item.param.type == ParamType::FLOAT){
                strcpy(item.param.show_fmt,weights_LIST_PARAM_SHOW_FLOAT);
            }
        }
        

        return TreeNode::createNode((const void*)&item,sizeof(item));
    }


private:
    ItemDataCreateItemConf conf;
    Param param;

    



};





void printItemData(unsigned char* data, size_t data_size);





}



extern sLM::sLittleMenu menu;



