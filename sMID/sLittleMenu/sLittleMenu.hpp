/**
 * sLittleMenu.cpp
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
 * v1.6 
 * todo 加入CANVAS支持,实现一个简易的动画渲染器
 * 
 * 
 * 
 * 
 * 
 */

#pragma once


#include "sLM_Conf.hpp"

#include "item/sLM_EnterableItem.hpp"
#include "sLM_Namespace.hpp"




namespace sLM{

enum class ItemType{
    ENTERABLE = 0,
    BUTTON = 1,
    LABEL = 2,
    SWITCH = 3,
    BAR = 4,
};




class sLittleMenu{
public:

    enum class ItemShowType{
        LIST = 0,
        PAGE,
        CANVAS
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

    EnterableItem* createEnterable(ItemBase* parent,const char* tittle);


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



};




//Item基类
class ItemBase{
public:
    ItemBase() = default;
    virtual ~ItemBase() = default;

    /*普通树*/
    ItemBase* parent;
    ItemBase* child;
    ItemBase* prev_sibling;
    ItemBase* next_sibling;

    //控件id
    uint32_t id;

    virtual ItemType getItemType() const = 0;

    virtual void operate_enter(sLittleMenu& menu) = 0;
    virtual void operate_back(sLittleMenu& menu) = 0;
    virtual void operate_prev(sLittleMenu& menu) = 0;
    virtual void operate_next(sLittleMenu& menu) = 0;

    int bindParent(ItemBase* parent);


};

class EnterableItem : public ItemBase{
public:
    ItemType getItemType() const override{
        return ItemType::ENTERABLE;
    }

    void operate_enter(sLittleMenu& menu) override;
    void operate_back(sLittleMenu& menu) override;
    void operate_prev(sLittleMenu& menu) override;
    void operate_next(sLittleMenu& menu) override;

    char tittle[SLM_ITEM_TEXT_LEN];


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



