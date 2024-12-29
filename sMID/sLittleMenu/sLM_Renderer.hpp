#pragma once


namespace sLM{

class TreeNode; //前向声明

class Renderer{
public:
    Renderer() = default;
    virtual ~Renderer() = default;

    // 纯虚函数：绘制菜单列表
    virtual void showMenuList(TreeNode* parent) = 0;

    virtual void showWatingDialog(const char* _title, const char* _message) = 0;

    virtual void update() = 0;
};

}


