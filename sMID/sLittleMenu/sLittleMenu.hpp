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

#include "sLittleMenu_Tree.hpp"





class sLittleMenu{
public:
    enum class NavEvent{
        NONE = 0,
        UP,
        DOWN,
        ENTER,
        BACK,
    };

    sLittleMenu();
    ~sLittleMenu();

    void init();

    sLM_Menu* createMenu(const void* src_data, size_t dsize);
    int addSub(sLM_Menu* submenu);

    void opEnter();
    void opBack();
    void opPrev();
    void opNext();

    void update();

    sLM_Menu* menu = nullptr;
    sLM_TreeNode* curr_menu = nullptr;

private:
    



};



