#include "sLM_Tree.hpp"

/**
 * sLM_Tree.cpp
 * Sightseer's Little Menu Tree
 * 适用于菜单的普通树的通用数据结构,基于柔性数组实现节点携带的内容任意改变
 * 
 * v1.0 241209 BySightseer.inHNIP9607
 * 实现了一个抽象的普通树数据结构,给sLittleMenu继承用
 * 
 */



//虚析构,保证删除此节点时,会递归的删除所有子节点和兄弟节点
sLM_TreeNode::~sLM_TreeNode(){
    delete child;
    delete next_sibling;
}

//静态函数,用于创建一个TreeNode,并将src_data拷贝到data[](深拷贝)
sLM_TreeNode* sLM_TreeNode::createNode(const void* src_data, size_t dsize){
    //分配足够的内存 sizeof(TreeNode)包含了data[1]，需要额外(dsize-1)来补足所需空间
    void* mem = pvPortMalloc(sizeof(sLM_TreeNode) + dsize - 1);
    if (!mem) return nullptr;
    //在已分配的内存上构造对象
    sLM_TreeNode* node = new(mem) sLM_TreeNode(dsize);
    //拷贝数据
    if(src_data != nullptr && dsize > 0)memcpy(node->data, src_data, dsize);
    return node;
}

int sLM_TreeNode::addChild(sLM_TreeNode* newChild){
    if (!newChild) return -1;
        //你的爸爸是我
        newChild->parent = this;
        //如果当前的节点还没有任何子节点,说明这是第一个,就直接赋值
        if (!child) {
            child = newChild;
        }
        //你已经不是第一个了
        else {
            sLM_TreeNode* temp = child;
            //找到最后一个节点(sibling为nullptr)
            while (temp->next_sibling)temp = temp->next_sibling;
            //连接上最后一个节点
            temp->next_sibling = newChild;
            //设置新子节点的上一个节点
            newChild->prev_sibling = temp;
        }
    return 0;
}

//调试用
void sLM_TreeNode::printTree(int level, void (*printFunc)(unsigned char*, size_t)){
        for (int i = 0; i < level; ++i) {
            sBSP_UART_Debug_Printf("  ");
        }
        printFunc(data, data_size);
        if (child) {
            child->printTree(level + 1, printFunc);
        }
        if (next_sibling) {
            next_sibling->printTree(level, printFunc);
        }
    }














