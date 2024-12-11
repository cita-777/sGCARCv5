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



//基础树节点类
class sLM_TreeNode{
protected:
    //空实现
    sLM_TreeNode(){}
    //构造函数设为protected 防止直接new 需要使用createNode分配内存
    sLM_TreeNode(size_t dsize) : \
    parent(nullptr),child(nullptr),prev_sibling(nullptr),next_sibling(nullptr),data_size(dsize){}

public:
    sLM_TreeNode* parent;       //指向父级节点
    sLM_TreeNode* child;        //指向当前节点的子节点
    sLM_TreeNode* prev_sibling; //上一个兄弟
    sLM_TreeNode* next_sibling; //指向同级的下一个兄弟节点
    size_t data_size;           //这个节点携带的数据节点的大小
    uint8_t data[1];            //柔性数组 实际分配时可变长度

    sLM_TreeNode(const sLM_TreeNode&) = delete;            //禁用复制
    sLM_TreeNode& operator=(const sLM_TreeNode&) = delete; //禁用移动
    virtual ~sLM_TreeNode();                               //虚析构

    //创建一个节点
    static sLM_TreeNode* createNode(const void* src_data, size_t dsize);
    //添加子节点
    int addChild(sLM_TreeNode* newChild);

    //打印树,需用户提供一个打印函数用于打印data内容
    void printTree(int level, void (*printFunc)(unsigned char*, size_t));
};



