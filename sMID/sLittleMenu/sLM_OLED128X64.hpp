#pragma once


#include "sLM_Renderer.hpp"

#include "sG2D.hpp"

//前向声明 在命名空间sLM里有这个类
namespace sLM{
    class sLittleMenu;


    const uint16_t W_MAX = 128u;
    const uint16_t H_MAX = 64u;



    class OLED128X64 : public Renderer{
    public:
        OLED128X64(sG2D* _screen,sLittleMenu* _menu);
        ~OLED128X64() override;

        //实现基类接口
        void showMenuList(TreeNode* parent) override;
        //一个让用户等待的对话框
        void showWatingDialog(const char* _title, const char* _message) override;

        void update() override;

    private:
        sG2D* screen;
        sLittleMenu* menu;

    };

}





