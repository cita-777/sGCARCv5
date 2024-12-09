#include "sLittleMenu.hpp"






sLittleMenu::sLittleMenu(){

}

sLittleMenu::~sLittleMenu(){

}


sLittleMenu::Item_ParamCbRetType sLittleMenu::param_change(void* param,Item_ParamType type,Item_ParamChrgDir dir){
    if(type == Item_ParamType::INT){
        int* p = (int*)param;
        if(dir == Item_ParamChrgDir::UP){
            *p = *p + 1;
        }
        else if(dir == Item_ParamChrgDir::DN){
            *p = *p + -1;
        }
    }
    return Item_ParamCbRetType::OK;
}


void sLittleMenu::init(){
    MenuItemData root_item = {.id = 0,.text = "root_menu",\
                            .child_show_mode = sLM::MenuChildShowMode::LIST,.is_hover = true};
    root = sLM_TreeNode::createNode((const void*)&root_item,sizeof(root_item));
    curr_page = root;
}

int sLittleMenu::addSubMenu(sLM_TreeNode* parent, sLM_TreeNode* child){
    if (!parent && !child) return -1;
    //添加到父节点的子节点列表
    return parent->addChild(child);
}

sLM::MenuItemData& sLittleMenu::getNodeData(){
    sLM::MenuItemData* item = reinterpret_cast<sLM::MenuItemData*>(curr_page->data);
    return *item;
}

sLM::MenuItemData& sLittleMenu::getNodeData(sLM_TreeNode* node){
    sLM::MenuItemData* item = reinterpret_cast<sLM::MenuItemData*>(node->data);
    return *item;
}

sLM_TreeNode& sLittleMenu::getFristNode(){
    sLM_TreeNode* node = curr_page; //从当前节点开始
    //沿着prev_sibling向上遍历直到找到最顶层节点
    while (node->prev_sibling) {
        node = node->prev_sibling;
    }
    return *node; //返回最上层节点
}




void sLittleMenu::opEnter(){
    sLM::MenuItemData& item = getNodeData(curr_page);
    //如果当前页面有子页面
    if(curr_page->child){
        //让旧的节点取消hover
        item.is_hover = false;
        //跳转到子页面
        curr_page = curr_page->child;
        //让新的节点hover
        getNodeData().is_hover = true;
    }
    //没有下一级了,说明要选中
    else{
        //选中项
        item.is_selected = true;
    }
    
    changeHandler(OpEvent::ENTER);
}

void sLittleMenu::opBack(){
    sLM::MenuItemData& item = getNodeData(curr_page);
    //如果当前项被选中了,就退出选中
    if(item.is_selected == true){
        item.is_selected = false;
    }
    //如果没有被选中,则跳转到上一级
    else if(curr_page->parent){
        //让旧的节点取消hover
        item.is_hover = false;
        curr_page = curr_page->parent;
        //让新的节点hover
        getNodeData().is_hover = true;
    }
    
    changeHandler(OpEvent::BACK);
}

void sLittleMenu::opPrev(){
    sLM::MenuItemData& item = getNodeData(curr_page);
    //如果已经被选中了就修改参数
    if(item.is_selected == true){
        increaseParam(item);
    }
    //没有选中就上下移动
    else if(curr_page->prev_sibling){
        //让旧的节点取消hover
        item.is_hover = false;
        curr_page = curr_page->prev_sibling;
        //让新的节点hover
        getNodeData(curr_page).is_hover = true;
    }
    
    changeHandler(OpEvent::PREV);
}

void sLittleMenu::opNext(){
    sLM::MenuItemData& item = getNodeData(curr_page);
    //如果已经被选中了就修改参数
    if(item.is_selected == true){
        decreaseParam(item);
    }
    //没有选中就上下移动
    else if(curr_page->next_sibling){
        //让旧的节点取消hover
        item.is_hover = false;
        curr_page = curr_page->next_sibling;
        //让新的节点hover
        getNodeData(curr_page).is_hover = true;
    }

    changeHandler(OpEvent::NEXT);
}


void sLittleMenu::increaseParam(MenuItemData& item){
    //float情况
    if(item.param.type == sLM::Item_ParamType::FLOAT){
        //先加再限幅
        item.param.f_val += item.param.f_increment;
        //限幅处理
        if(item.param.lim_type == Item_ParamLimitType::NO_LIMIT){
            return;
        }
        else if(item.param.lim_type == Item_ParamLimitType::MAX_LIMIT || 
                item.param.lim_type == Item_ParamLimitType::RANGE){
            //向上限幅
            if(item.param.f_val > item.param.f_up_val){
                item.param.f_val = item.param.f_up_val;
            }
            return;
        }
    }
    else if(item.param.type == sLM::Item_ParamType::INT){
        item.param.i_val += item.param.i_increment;
        //限幅处理
        if(item.param.lim_type == Item_ParamLimitType::NO_LIMIT){
            return;
        }
        else if(item.param.lim_type == Item_ParamLimitType::MAX_LIMIT || 
                item.param.lim_type == Item_ParamLimitType::RANGE){
            //向上限幅
            if(item.param.i_val > item.param.i_up_val){
                item.param.i_val = item.param.i_up_val;
            }
            return;
        }
    }
}

void sLittleMenu::decreaseParam(MenuItemData& item){
    //float情况
    if(item.param.type == sLM::Item_ParamType::FLOAT){
        //先减再限幅
        item.param.f_val -= item.param.f_decrement;
        //限幅处理
        if(item.param.lim_type == Item_ParamLimitType::NO_LIMIT){
            return;
        }
        else if(item.param.lim_type == Item_ParamLimitType::MIN_LIMIT || 
                item.param.lim_type == Item_ParamLimitType::RANGE){
            //向下限幅
            if(item.param.f_val < item.param.f_dn_val){
                item.param.f_val = item.param.f_dn_val;
            }
            return;
        }
    }
    else if(item.param.type == sLM::Item_ParamType::INT){
        item.param.i_val -= item.param.i_decrement;
        //限幅处理
        if(item.param.lim_type == Item_ParamLimitType::NO_LIMIT){
            return;
        }
        else if(item.param.lim_type == Item_ParamLimitType::MIN_LIMIT || 
                item.param.lim_type == Item_ParamLimitType::RANGE){
            //向下限幅
            if(item.param.i_val < item.param.i_dn_val){
                item.param.i_val = item.param.i_dn_val;
            }
            return;
        }
    }
}

sLM_TreeNode* sLittleMenu::getCurrMenu(){
    return curr_page;
}

void sLittleMenu::resetToRoot(){
    curr_page = root;
}


void sLittleMenu::changeHandler(OpEvent ev){
    sLM::MenuItemData& curr_item = getNodeData();
    oled.setAll(0);
    sLM_Show::showList(curr_page->parent);
    oled.handler();
    if(ev == OpEvent::ENTER){

    }

}

void sLittleMenu::update(){

}



// 自定义打印函数，用于打印MenuItemData
void printMenuItemData(const unsigned char* data, size_t data_size) {
    const sLM::MenuItemData* item = reinterpret_cast<const sLM::MenuItemData*>(data);
    sBSP_UART_Debug_Printf("- ID: %u,text:%s",item->id,item->text);

    if(item->param.type == sLM::Item_ParamType::STRING){
        sBSP_UART_Debug_Printf(", String: %s ",item->param.str_val);
    }
    else if(item->param.type == sLM::Item_ParamType::FLOAT){
        sBSP_UART_Debug_Printf(", Float: %f ",item->param.f_val);
    }
    else if(item->param.type == sLM::Item_ParamType::INT){
        sBSP_UART_Debug_Printf(", Int32_Num: %d ",item->param.i_val);
    }
    sBSP_UART_Debug_Printf(", selected:%u,is_hover:%u\n",item->is_selected,item->is_hover);
}
