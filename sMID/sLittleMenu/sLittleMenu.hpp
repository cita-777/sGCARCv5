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


#define SLM_ITEM_TEXT_LEN 16
//item的显示的字符串参数最大的长度(byte),不能小于int的长度(4)
#define SLM_MAX_ISP_STR_LEN 16


// enum class 

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


class sLittleMenu{
public:
    enum class NavEvent{
        NONE = 0,
        UP,
        DOWN,
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

    void opEnter();
    void opBack();
    void opPrev();
    void opNext();

    void update();


private:


};



void printMenuItemData(const unsigned char* data, size_t data_size);

