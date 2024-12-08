#include "sLittleMenu_Tree.hpp"






//虚析构,保证删除此节点时,会递归的删除所有子节点和兄弟节点
sLM_TreeNode::~sLM_TreeNode(){
    delete child;
    delete next_sibling;
    delete prev_sibling;
}

//静态函数,用于创建一个TreeNode,并将src_data拷贝到data[]
sLM_TreeNode* sLM_TreeNode::createNode(const void* src_data, size_t dsize){
    // 分配足够的内存：
    // sizeof(TreeNode)包含了data[1]，需要额外(dsize-1)来补足所需空间
    void* mem = pvPortMalloc(sizeof(sLM_TreeNode) + dsize - 1);
    if (!mem) {
        return nullptr;
    }
    // 在已分配的内存上构造对象（placement new）
    sLM_TreeNode* node = new(mem) sLM_TreeNode(dsize);
    // 拷贝数据
    memcpy(node->data, src_data, dsize);
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
            while (temp->next_sibling) {
                temp = temp->next_sibling;
            }
            //连接上最后一个节点
            temp->next_sibling = newChild;
        }
    return 0;
}

void sLM_TreeNode::printTree(int level, void (*printFunc)(const unsigned char*, size_t)) const {
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


sLM_Menu* sLM_Menu::create(const void* src_data, size_t dsize){
    sLM_TreeNode* base = sLM_TreeNode::createNode(src_data, dsize);
    // 强制转换为Menu*
    // 注：这里无新增成员变量，内存布局与sLM_TreeNode一致，转换安全性依赖设计约定
    return static_cast<sLM_Menu*>(base);
}

void sLM_Menu::addSubMenu(sLM_Menu* submenu){
    addChild(submenu);
}

void sLM_Menu::printMenu(void (*printFunc)(const unsigned char*, size_t)) const {
    printTree(0, printFunc);
}


// 自定义打印函数，用于打印MenuItemData
void printMenuItemData(const unsigned char* data, size_t data_size) {
    if (data_size != sizeof(sLM_MenuItemData)) {
        sBSP_UART_Debug_Printf("- [Unknown data size]\n");
        return;
    }
    const sLM_MenuItemData* item = reinterpret_cast<const sLM_MenuItemData*>(data);
    sBSP_UART_Debug_Printf("- ID: %u,text:%s",item->id,item->text);
    if(item->show_para_type == sLM_ISPType::STRING){
        sBSP_UART_Debug_Printf(", String: %s \n",item->para_str);
    }
    else if(item->show_para_type == sLM_ISPType::FLOAT){
        sBSP_UART_Debug_Printf(", Float: %f \n",item->para_float);
    }
    else if(item->show_para_type == sLM_ISPType::INT32_NUM){
        sBSP_UART_Debug_Printf(", Int32_Num: %d \n",item->para_int);
    }
}










