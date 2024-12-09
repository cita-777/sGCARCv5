#include "sLittleMenu.hpp"






sLittleMenu::sLittleMenu(){

}

sLittleMenu::~sLittleMenu(){

}


void sLittleMenu::init(){
    sLM_MenuItemData root_data = {.id = 0,\
                                .text = "This is root",\
                                .show_para_type = sLM_ISPType::STRING,\
                                .para_str = "root str"\
                                };
    root = sLM_TreeNode::createNode((const void*)&root_data,sizeof(root_data));
    curr_page = root;
}

int sLittleMenu::addSubMenu(sLM_TreeNode* parent, sLM_TreeNode* child){
    if (!parent && !child) return -1;
    //添加到父节点的子节点列表
    return parent->addChild(child);
}


void sLittleMenu::opEnter(){
    if(curr_page->child){
        curr_page = curr_page->child;
    }
}

void sLittleMenu::opBack(){
    if(curr_page->parent){
        curr_page = curr_page->parent;
    }
}

void sLittleMenu::opPrev(){
    if(curr_page->prev_sibling){
        curr_page = curr_page->prev_sibling;
    }
}

void sLittleMenu::opNext(){
    if(curr_page->next_sibling){
        curr_page = curr_page->next_sibling;
    }
}

sLM_TreeNode* sLittleMenu::getCurrMenu(){
    return curr_page;
}

void sLittleMenu::resetToRoot(){
    curr_page = root;
}

void sLittleMenu::update(){

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
