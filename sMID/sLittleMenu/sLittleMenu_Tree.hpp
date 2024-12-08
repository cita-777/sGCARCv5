#pragma once



#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"
#include <string>
#include <cstring>

#include "FreeRTOS.h"
#include "task.h"

#include "sBSP_UART.h"



#define SLM_ITEM_TEXT_LEN 16
//item的显示的字符串参数最大的长度(byte),不能小于int的长度(4)
#define SLM_MAX_ISP_STR_LEN 16


//item显示的参数的类型,ItemShowParaType
enum class sLM_ISPType{
    INT32_NUM = 0,
    INT32_HEX = 1,
    UINT32_NUM = 2,
    UINT32_HEX = 3,
    FLOAT = 4,
    CHAR_HEX = 5,
    CHAR_ASCII = 6,
    STRING = 7,
    STRING_HEX = 8,
};

//item的数据结构体
struct sLM_MenuItemData{
    uint32_t id;    //唯一id
    char text[SLM_ITEM_TEXT_LEN];    //显示的文本
    sLM_ISPType show_para_type;    //显示的参数的类型
    union{  //储存显示参数
        int32_t para_int;
        uint32_t para_uint;
        float para_float;
        char para_char;
        char para_str[SLM_MAX_ISP_STR_LEN];
    };
};

// 基础树节点类
class sLM_TreeNode{
protected:

    //构造函数设为protected 防止直接new 需要使用createNode分配内存
    sLM_TreeNode(size_t dsize) : \
    parent(nullptr),child(nullptr),prev_sibling(nullptr),next_sibling(nullptr),data_size(dsize){}

public:
    sLM_TreeNode* parent;   //指向父级节点
    sLM_TreeNode* child;    //指向当前节点的子节点
    sLM_TreeNode* prev_sibling; //上一个兄弟
    sLM_TreeNode* next_sibling;  //指向同级的下一个兄弟节点
    size_t data_size;   //这个节点携带的数据节点的大小
    uint8_t data[1];    //柔性数组 实际分配时可变长度

    sLM_TreeNode(const sLM_TreeNode&) = delete; //禁用复制
    sLM_TreeNode& operator=(const sLM_TreeNode&) = delete; //禁用移动

    virtual ~sLM_TreeNode();

    static sLM_TreeNode* createNode(const void* src_data, size_t dsize);

    // 添加子节点
    int addChild(sLM_TreeNode* newChild);

    // 打印树（需用户提供一个打印函数用于打印data内容）
    void printTree(int level, void (*printFunc)(const unsigned char*, size_t)) const;
};


// 菜单类，继承自TreeNode
class sLM_Menu : public sLM_TreeNode {
    // 私有构造，必须通过create创建
    sLM_Menu(size_t dsize) : sLM_TreeNode(dsize) {}

public:
    // 创建菜单节点，传入自定义数据结构
    static sLM_Menu* create(const void* src_data, size_t dsize);

    // 添加子菜单
    void addSubMenu(sLM_Menu* submenu);

    // 打印菜单
    void printMenu(void (*printFunc)(const unsigned char*, size_t)) const;
};



void printMenuItemData(const unsigned char* data, size_t data_size);


