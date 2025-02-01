/**
 * sLittleMenu.hpp
 * Sightseer's LittleMenu 一个轻量级的菜单~
 * 旨在实现在调参场景快速地进行菜单编写
 * 实现了一个轻量级的菜单,此文件只负责菜单的结构管理,不负责显示,以此实现高通用性
 * 
 * v1.0 241211Night BySightseer.inHNIP9607
 * 第一版完工! demo做好了
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
 * v1.4 250114 bySightseer.
 * 加入了最大菜单项数量限制,默认128,维护一个线性的查询数组
 * 加入了模拟操作,可以模拟用户对菜单的操作
 * 
 * v1.5 250122 bySightseer.
 * 优化代码架构,优化了lock,采用全局lock.
 * 
 * v1.6 250201 bySightseer.
 * 使用多态重写整个sLittleMenu,实现了CANVAS功能,重构了sLittleMenu和OLED128X64渲染器
 * 优化了菜单项创建流程,取消查询数组,没有菜单项限制
 * 
 * 
 * 
 * 
 */

#pragma once


#include "sLM_Conf.hpp"


#include <string>

//placement new创建类
#define SLM_CREATE_CLASS(_ITEM_NAME,_PARAM,...) new(SLM_PORT_MALLOC(sizeof(_ITEM_NAME))) _ITEM_NAME(_PARAM);


namespace sLM{
/*前向声明*/
class sLittleMenu;
class Renderer;

/*Item类*/
class ItemBase;
class EnterableItem;
class LabelItem;
class ButtonItem;
class IntValAdj;
class FloatValAdj;
class SwitchItem;
class BarItem;

//子菜单显示方式
enum class ItemShowType{
    LIST = 0,
    PAGE,
    CANVAS
};

//Item类型
enum class ItemType{
    ENTERABLE = 0,
    BUTTON = 1,
    LABEL = 2,
    SWITCH = 3,
    BAR = 4,
    INT_VAL_ADJ = 5,
    FLOAT_VAL_ADJ = 6,
};

//约束类型
enum class ConstraintType{
    NONE = 0,
    MAX,
    MIN,
    RANGE,
};

//回调时机
enum class CallBackMethod{
    NON = 0,    //不回调
    EXIT,       //退出时回调
    CHANGE,     //修改时就回调
};

using ButtonPressCb = void(*)(ItemBase* item,uint32_t id);
using IntValChangeCb = void(*)(IntValAdj* item,uint32_t id,int value);
using FloatValChangeCb = void(*)(FloatValAdj* item,uint32_t id,float value);
using SwitchPressCb = void(*)(SwitchItem* item,uint32_t id,bool status);

using CanvasEnterCb = void(*)(EnterableItem* parent_item,uint32_t id);
using CanvasPeriodicallyCb = void(*)(EnterableItem* parent_item,uint32_t id);
using CanvasExitCb  = void(*)(EnterableItem* parent_item,uint32_t id);


class sLittleMenu{
public:

    
    
    //操作事件类型
    enum class OpEvent{
        NONE = 0,
        PREV,
        NEXT,
        ENTER,
        BACK,
        LOCK,
    };

    




    int init(Renderer* _renderer);
    int init();


    //获取同级的第一个节点
    ItemBase& getFristNode();
    //使用任意兄弟节点获取同级的第一个节点
    ItemBase& getFristNode(ItemBase* any_sibling);
    //获取当前菜单项的同级的索引,从1开始
    uint32_t getCurrMenuIndex();
    //获取当前菜单项同级有多少个菜单
    uint32_t getCurrMenuNumber();
    //返回当前同级的第多少个菜单项
    ItemBase* getIndexMenu(uint32_t index);
    //返回任意兄弟菜单的索引
    ItemBase* getIndexMenu(ItemBase* any_slibing,uint32_t index);



    void operateEnter();
    void operateBack();
    void operatePrev();
    void operateNext();

    void update();

    ItemBase* getCurr(){
        return curr;
    }


    void printAllItem();


    ItemBase* curr;
    ItemBase* home;

    //关于菜单被锁定的信息
    struct LockInfo{
        bool status;
        char tittle[SLM_LOCK_TITTLE_LEN];
        char message[SLM_LOCK_MESSAGE_LEN];
        LockInfo(){tittle[0] = '\0';message[0] = '\0';}
    };

    LockInfo lock_info;

private:
    
    Renderer* renderer;
    OpEvent op_event;

    void printAllItemRecursively(ItemBase* item, int level, const std::string& prefix);



};




//Item基类
class ItemBase{
public:
    
    virtual ~ItemBase() = default;

    /*普通树*/
    ItemBase* parent = nullptr;
    ItemBase* child = nullptr;
    ItemBase* prev_sibling = nullptr;
    ItemBase* next_sibling = nullptr;

    //控件id
    uint32_t id = 0;

    bool is_selected = false;
    bool is_hover    = false;

    virtual ItemType getItemType() const = 0;
    virtual const char* getTittle() const = 0;

    //菜单默认操作
    virtual void operate_enter(sLittleMenu& menu);
    virtual void operate_back(sLittleMenu& menu);
    virtual void operate_prev(sLittleMenu& menu);
    virtual void operate_next(sLittleMenu& menu);

    int bindParent(ItemBase* parent);

    virtual void print() const = 0;

protected:
    //不允许外部直接创建
    ItemBase() = default;

};


//可进入的菜单项
class EnterableItem : public ItemBase{
public:
    //用于确认item是什么类型
    ItemType getItemType() const override{return ItemType::ENTERABLE;}

    void print() const override{
        SLM_PRINTF("id=%u,Enterable:%s,H=%d\n",id,tittle,is_hover?1:0);
    }

    //创建方式
    static EnterableItem& create(ItemBase* parent,uint32_t _id);
    EnterableItem& setTittle(const char* tittle);
    EnterableItem& setChildShowType(ItemShowType type);

    EnterableItem& setCanvasEnterCallback(CanvasEnterCb enter_cb){
        canvas_enter_callback = enter_cb;
        return *this;
    }
    EnterableItem& setCanvasPeriodicallyCallback(CanvasPeriodicallyCb cb){
        canvas_periodically_callback = cb;
        return *this;
    }
    EnterableItem& setCanvasExitCallback(CanvasExitCb exit_cb){
        canvas_exit_callback = exit_cb;
        return *this;
    }

    /*重载操作方式*/
    void operate_enter(sLittleMenu& menu) override;
    void operate_back (sLittleMenu& menu) override;
    void operate_prev (sLittleMenu& menu) override;
    void operate_next (sLittleMenu& menu) override;

    const char* getTittle() const override{
        return tittle;
    }

    ItemShowType getChildShowType() const{
        return child_show_type;
    }

    void CallCanvasPeriodciallyCallback(){
        canvas_periodically_callback(static_cast<EnterableItem*>(this),id);
    }

private:
    //默认以list方式显示
    ItemShowType child_show_type = ItemShowType::LIST;
    //标题字
    char tittle[SLM_ITEM_TEXT_LEN];
    CanvasEnterCb canvas_enter_callback = nullptr;
    CanvasPeriodicallyCb canvas_periodically_callback = nullptr;
    CanvasExitCb canvas_exit_callback = nullptr;
};


//标签
class LabelItem : public ItemBase{
public:
    ItemType getItemType() const override{return ItemType::LABEL;}

    //覆写按下enter的操作
    void operate_enter(sLittleMenu& menu) override{
        SLM_LOG_WARN("ENTER操作失败,Label不可进入");
    }

    void print() const override{
        SLM_PRINTF("id=%u,Label:%s,H=%d\n",id,tittle,is_hover?1:0);
    }

    //创建方式
    static LabelItem& create(ItemBase* parent,uint32_t _id);
    LabelItem& setTittle(const char* tittle);

    const char* getTittle() const override{
        return tittle;
    }

private:
    LabelItem(){
        tittle[0] = '\0';
    }

    char tittle[SLM_ITEM_TEXT_LEN];
};

//按钮
class ButtonItem : public ItemBase{
public:
    ItemType getItemType() const override{return ItemType::BUTTON;}

    //按钮控件,按下Enter则调用回调,不可选中
    void operate_enter(sLittleMenu& menu) override{
        if(press_callback){
            press_callback(this,id);
        }
    }

    void print() const override{
        SLM_PRINTF("id=%u,Button:%s,cover:%s,H=%d\n",id,tittle,cover_text,is_hover?1:0);
    }

    //创建方式
    static ButtonItem& create(ItemBase* parent,uint32_t _id);
    ButtonItem& setContext(const char* tittle,const char* cover_text);
    ButtonItem& setCallback(ButtonPressCb press_cb);

    const char* getTittle() const override{
        return tittle;
    }

    const char* getCoverText(){
        return cover_text;
    }


private:
    //按钮标题
    char tittle[SLM_ITEM_TEXT_LEN];
    //按钮上的文本
    char cover_text[SLM_BUTTON_COVER_TEXT_LEN];
    //按下回调
    ButtonPressCb press_callback = nullptr;

    ButtonItem(){
        tittle[0] = '\0';
        cover_text[0] = '\0';
    }
};

class SwitchItem : public ItemBase{
public:
    ItemType getItemType() const override{return ItemType::SWITCH;}

    //按钮控件,按下Enter则调用回调,不可选中
    void operate_enter(sLittleMenu& menu) override{
        status = !status;
        if(press_callback){
            press_callback(this,id,status);
        }
    }

    void print() const override{
        SLM_PRINTF("id=%u,Switch:%s,Status:%s,H=%d\n",id,tittle,status?on_text:off_text,is_hover?1:0);
    }

    //创建方式
    static SwitchItem& create(uint32_t _id);
    static SwitchItem& create(ItemBase* parent,uint32_t _id);
    SwitchItem& setContext(const char* tittle);
    SwitchItem& setContext(const char* tittle,const char* on_text,const char* off_text);
    SwitchItem& setCallback(SwitchPressCb press_cb);
    SwitchItem& setStatus(bool status);

    const char* getTittle() const override{
        return tittle;
    }

    const char* getText() const{
        return status ? on_text : off_text;
    }

    bool getStatus() const{
        return status;
    }

private:
    //标题
    char tittle[SLM_ITEM_TEXT_LEN];
    //拨到ON位置时文本
    char on_text[SLM_SWITCH_TEXT_LEN];
    char off_text[SLM_SWITCH_TEXT_LEN];
    //回调
    SwitchPressCb press_callback = nullptr;
    bool status = false;

    SwitchItem(uint32_t id){
        tittle[0] = '\0';
        //设置默认值
        strncpy(on_text,SLM_SWITCH_ON_TEXT_DEFAULT,SLM_SWITCH_TEXT_LEN);
        strncpy(off_text,SLM_SWITCH_OFF_TEXT_DEFAULT,SLM_SWITCH_TEXT_LEN);
    }
};

class BarItem : public ItemBase{
public:


};


class IntValAdj : public ItemBase{
public:
    //用户使用create来创建一个int类型的数值调整项
    static IntValAdj& create(ItemBase* parent,uint32_t _id);
    IntValAdj& setContext(const char* tittle,const char* show_fmt,int default_val,int increment,int decrement);
    IntValAdj& setCallback(IntValChangeCb change_cb,CallBackMethod cb_method);
    IntValAdj& setConstraint(ConstraintType cons_type,int max,int min);
    
    //获取item类型
    ItemType getItemType() const override{return ItemType::INT_VAL_ADJ;}
    /*重载操作方式*/
    void operate_enter(sLittleMenu& menu) override;
    void operate_back (sLittleMenu& menu) override;
    void operate_prev (sLittleMenu& menu) override;
    void operate_next (sLittleMenu& menu) override;
    
    /*重载print方法*/
    void print() const override;

    //获取标题
    const char* getTittle() const override{return tittle;}
    //获取值的文本
    const char* getValText() const;

private:
    IntValAdj(){
        tittle[0] = '\0';
        value_text[0] = '\0';
        show_fmt[0] = '\0';
    }

    //值
    int value = 0;
    //增减量
    int increment = 1;int decrement = 1;
    //阈值
    int max = 100;int min = -100;
    //约束类型
    ConstraintType cons_t = ConstraintType::NONE;
    //被修改时的回调
    IntValChangeCb change_callback = nullptr;
    //调用回调的时机
    CallBackMethod callback_method = CallBackMethod::CHANGE;
    //显示的标题
    char tittle[SLM_ITEM_TEXT_LEN];
    //数值的文本
    char value_text[SLM_INT_VAL_ADJ_VAL_LEN];
    //格式化显示的方式
    char show_fmt[SLM_INT_VAL_ADJ_FMT_LEN];

    //处理增量,参数:1为加,0为减
    void incDecProcess(bool is_inc);
};

class FloatValAdj : public ItemBase{
public:
    //用户使用create来创建一个int类型的数值调整项
    static FloatValAdj& create(ItemBase* parent,uint32_t _id);
    FloatValAdj& setContext(const char* tittle,const char* show_fmt,float default_val,float increment,float decrement);
    FloatValAdj& setCallback(FloatValChangeCb change_cb,CallBackMethod cb_method);
    FloatValAdj& setConstraint(ConstraintType cons_type,float max,float min);
    
    //获取item类型
    ItemType getItemType() const override{return ItemType::FLOAT_VAL_ADJ;}
    /*重载操作方式*/
    void operate_enter(sLittleMenu& menu) override;
    void operate_back (sLittleMenu& menu) override;
    void operate_prev (sLittleMenu& menu) override;
    void operate_next (sLittleMenu& menu) override;
    
    /*重载print方法*/
    void print() const override;

    //获取标题
    const char* getTittle() const override{return tittle;}
    //获取值的文本
    const char* getValText() const;

private:
    FloatValAdj(){
        tittle[0] = '\0';
        value_text[0] = '\0';
        show_fmt[0] = '\0';
    }

    //值
    float value = 0.0f;
    //增减量
    float increment = 0.1f;float decrement = 0.1f;
    //阈值
    float max = 1.0f;float min = -1.0f;
    //约束类型
    ConstraintType cons_t = ConstraintType::NONE;
    //被修改时的回调
    FloatValChangeCb change_callback = nullptr;
    //调用回调的时机
    CallBackMethod callback_method = CallBackMethod::CHANGE;
    //显示的标题
    char tittle[SLM_ITEM_TEXT_LEN];
    //数值的文本
    char value_text[SLM_INT_VAL_ADJ_VAL_LEN];
    //格式化显示的方式
    char show_fmt[SLM_INT_VAL_ADJ_FMT_LEN];

    //处理增量,参数:1为加,0为减
    void incDecProcess(bool is_inc);
};





class Renderer{
public:
    Renderer() = default;
    virtual ~Renderer() = default;

    //纯虚函数：绘制菜单列表
    virtual void showMenuList(ItemBase* parent) = 0;
    virtual void showWatingDialog(const char* _title, const char* _message) = 0;
    virtual void update() = 0;
};






}


extern sLM::sLittleMenu menu;



