#include "sLittleMenu.hpp"



sLM::sLittleMenu menu;




namespace sLM{




int sLittleMenu::init(Renderer* _renderer){
    renderer = _renderer;
    home = menu.createEnterable(nullptr,SLM_ROOT_MENU_TEXT);
    curr = home;
    return 0;
}

int sLittleMenu::init(){

    home = menu.createEnterable(nullptr,SLM_ROOT_MENU_TEXT);
    curr = home;


    return 0;
}


//获取同级的第一个节点
ItemBase& sLittleMenu::getFristNode(){
    ItemBase* node = curr; //从当前节点开始
    //沿着prev_sibling向上遍历直到找到最顶层节点
    while(node->prev_sibling)node = node->prev_sibling;
    return *node; //返回最上层节点
}

//使用任意兄弟节点获取同级的第一个节点
ItemBase& sLittleMenu::getFristNode(ItemBase* any_sibling){
    ItemBase* node = any_sibling;
    //沿着prev_sibling向上遍历直到找到最顶层节点
    while (node->prev_sibling)node = node->prev_sibling;
    return *node; //返回最上层节点
}

//获取当前节点的同级的索引值
uint32_t sLittleMenu::getCurrMenuIndex(){
    uint32_t index = 1;
    ItemBase* node = curr;
    while(node->prev_sibling){
        node = node->prev_sibling;
        index++;
    }
    return index;
}

//获取当前菜单项同级有多少个菜单
uint32_t sLittleMenu::getCurrMenuNumber(){
    uint32_t count = 0;
    ItemBase* node = &getFristNode();
    while(node){
        node = node->next_sibling;
        count++;
    }
    return count;
}

//通过索引返回当前同级的菜单项
ItemBase* sLittleMenu::getIndexMenu(uint32_t index){
    ItemBase* node = &getFristNode();
    while(index-- > 1){
        node = node->next_sibling;
    }
    return node;
}

//返回任意兄弟菜单的索引
ItemBase* sLittleMenu::getIndexMenu(ItemBase* any_slibing ,uint32_t index){
    //找到最上面的节点
    ItemBase* node = &getFristNode(any_slibing);
    while(index-- > 1){
        node = node->next_sibling;
    }
    //再通过索引找到需要的菜单项
    node = getIndexMenu(index);
    return node;
}



void sLittleMenu::operateEnter(){
    if(op_event == OpEvent::NONE){
        op_event = OpEvent::ENTER;
    }else{
        SLM_LOG_WARN("发生了Enter事件,但是当前的操作事件不为空");
    }
}

void sLittleMenu::operateBack(){
    if(op_event == OpEvent::NONE){
        op_event = OpEvent::BACK;
    }else{
        SLM_LOG_WARN("发生了Back事件,但是当前的操作事件不为空");
    }
}

void sLittleMenu::operatePrev(){
    if(op_event == OpEvent::NONE){
        op_event = OpEvent::PREV;
    }else{
        SLM_LOG_WARN("发生了Prev事件,但是当前的操作事件不为空");
    }
}

void sLittleMenu::operateNext(){
    if(op_event == OpEvent::NONE){
        op_event = OpEvent::NEXT;
    }else{
        SLM_LOG_WARN("发生了Next事件,但是当前的操作事件不为空");
    }
}




void sLittleMenu::update(){
    renderer->update();
}







EnterableItem* sLittleMenu::createEnterable(ItemBase* parent,const char* tittle){
    void* mem = SLM_PORT_MALLOC(sizeof(EnterableItem));
    EnterableItem* item = new(mem) EnterableItem();    

    //绑定
    item->bindParent(parent);

    strncpy(item->tittle,tittle,SLM_ITEM_TEXT_LEN);
    return item;
}


int ItemBase::bindParent(ItemBase* parent){
    if(parent){
        if(!parent->child){
            parent->child = this;
            this->parent = parent;
        }
        else{
            ItemBase* temp = parent->child;
            //找到最后一个节点(sibling为nullptr)
            while(temp->next_sibling)temp = temp->next_sibling;
            //连接上最后一个节点
            temp->next_sibling = this;
            //设置新子节点的上一个节点
            this->prev_sibling = temp;
        }
    }
    return 0;
}




void EnterableItem::operate_enter(sLittleMenu& menu){
    if(menu.curr->child){
        menu.curr = menu.curr->child;
    }
}

void EnterableItem::operate_back(sLittleMenu& menu){
    if(menu.curr->parent){
        menu.curr = menu.curr->parent;
    }
}

void EnterableItem::operate_prev(sLittleMenu& menu){
    if(menu.curr->prev_sibling){
        menu.curr = menu.curr->prev_sibling;
    }
}

void EnterableItem::operate_next(sLittleMenu& menu){
    if(menu.curr->next_sibling){
        menu.curr = menu.curr->next_sibling;
    }
}






}


