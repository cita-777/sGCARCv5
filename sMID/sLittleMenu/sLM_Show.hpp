#pragma once

// #include "sLM_Tree.hpp"
// #include "sLM_Port.hpp"


#include "sG2D.hpp"

#include "sLittleMenu.hpp"

#include <cstring>
#include <stdio.h>


#define SLM_WEIGHTS_LIST_PARAM_SHOW_POS     80
//显示格式 eg:5%
#define SLM_WEIGHTS_LIST_PARAM_SHOW_INT     " %d%s"
#define SLM_WEIGHTS_LIST_PARAM_SHOW_FLOAT   " %.2f%s"


class sLM_OLEDWeights{
private:
    //列表最大显示项目数
    static const uint16_t LIST_MAX_ITEMS = 4;
    // static const char NEXT_ICON
    sG2D* screen;
    sLittleMenu* menu;

public:

    sLM_OLEDWeights();
    ~sLM_OLEDWeights();

    void init(sG2D* _screen,sLittleMenu* _menu);

    void showList(sLM_TreeNode* parent);


};


extern sLM_OLEDWeights slm_weights;




