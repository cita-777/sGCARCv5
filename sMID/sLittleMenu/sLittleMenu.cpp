#include "sLittleMenu.hpp"



sLM::sLittleMenu menu;




namespace sLM{




int sLittleMenu::init(Renderer* _renderer){
    renderer = _renderer;
    home = &EnterableItem::create(nullptr,0).setTittle(SLM_ROOT_MENU_TEXT);
    home->is_hover = true;
    curr = home;
    return 0;
}

int sLittleMenu::init(){
    home = &EnterableItem::create(nullptr,0).setTittle(SLM_ROOT_MENU_TEXT);
    home->is_hover = true;
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
    if(op_event == OpEvent::NONE){
        goto SHOW;
    }
    
    if(lock_info.status){
        SLM_LOG_ERR("菜单被锁定,无法操作");
        goto SHOW;
    }
    else if(tips_info.status){
        //? tips dialog和lock dialog的区别就是,tips dialog会在按下任意键后消失,lock dialog需要等待程序自己解锁
        tips_info.status = false;
        goto SHOW;
    }

    if(op_event == OpEvent::ENTER){
        SLM_LOG_INFO("发生了ENTER事件");
        curr->operate_enter(*this);
    }
    else if(op_event == OpEvent::BACK){
        SLM_LOG_INFO("发生了BACK事件");
        curr->operate_back(*this);
    }
    else if(op_event == OpEvent::PREV){
        SLM_LOG_INFO("发生了PREV事件");
        curr->operate_prev(*this);
    }
    else if(op_event == OpEvent::NEXT){
        SLM_LOG_INFO("发生了NEXT事件");
        curr->operate_next(*this);
    }

    // printAllItem();
    //处理完了,复位
SHOW:
    op_event = OpEvent::NONE;

    (void)0;
    renderer->update();
}

//调试用
void sLittleMenu::printAllItem(){
    printAllItemRecursively(home, 0, "├─");
}



// 辅助递归函数，用来遍历并打印所有菜单项
void sLittleMenu::printAllItemRecursively(ItemBase* item, int level, const std::string& prefix) {
    if (item == nullptr) return;  // 遇到空指针，直接返回

    // 打印当前菜单项的信息，并加入层级缩进
    // level 代表当前的菜单层级
    std::string indent(level * 4, ' ');  // 每一层级缩进 4 个空格
    std::string print_prefix = (prefix == "├─" || prefix == "└─") ? prefix : "│   "; // 控制连接符
    SLM_PRINTF("%s%s", indent.c_str(), print_prefix.c_str());
    item->print();

    // 如果当前菜单项有子项（child），递归遍历子项
    if (item->child) {
        printAllItemRecursively(item->child, level + 1, "├─");
    }

    // 遍历当前菜单项的兄弟节点（sibling），递归遍历下一个兄弟节点
    if (item->next_sibling) {
        printAllItemRecursively(item->next_sibling, level, "└─");
    }
}

void sLittleMenu::setLock(const char* tittle,const char* msg){
    lock_info.status = true;
    lock_info.tittle = tittle;
    lock_info.message = msg;
}

void sLittleMenu::setLock(){
    lock_info.status = true;
    lock_info.tittle = SLM_LOCK_DEFAULT_TITTLE;
    lock_info.message = SLM_LOCK_DEFAULT_MESSAGE;
}

void sLittleMenu::setUnlock(){
    lock_info.status = false;
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
            //新子节点的父节点为parent
            this->parent = parent;
        }
    }
    return 0;
}



void ItemBase::operate_enter(sLittleMenu& menu){
    if(menu.curr->child){
        menu.curr->is_hover = false;
        menu.curr = menu.curr->child;
        menu.curr->is_hover = true;
    }else{
        SLM_LOG_WARN("ENTER操作失败,子菜单为空");
    }
}

void ItemBase::operate_back(sLittleMenu& menu){
    //不能回到根节点
    if(menu.curr->parent->parent){
        menu.curr->is_hover = false;
        menu.curr = menu.curr->parent;
        menu.curr->is_hover = true;
    }else{
        SLM_LOG_WARN("BACK操作失败,父菜单为空");
    }
}

void ItemBase::operate_prev(sLittleMenu& menu){
    if(menu.curr->prev_sibling){
        menu.curr->is_hover = false;
        menu.curr = menu.curr->prev_sibling;
        menu.curr->is_hover = true;
        
    }else{
        SLM_LOG_WARN("PREV操作失败,上级菜单为空");
    }
}

void ItemBase::operate_next(sLittleMenu& menu){
    if(menu.curr->next_sibling){
        menu.curr->is_hover = false;
        menu.curr = menu.curr->next_sibling;
        menu.curr->is_hover = true;
    }else{
        SLM_LOG_WARN("NEXT操作失败,下级菜单为空");
    }
}


EnterableItem& EnterableItem::create(ItemBase* parent,uint32_t _id){
    EnterableItem* item = SLM_CREATE_CLASS(EnterableItem,);
    if(!item){
        SLM_LOG_ERR("malloc返回空指针,EnterableItem创建失败");
    }
    SLM_LOG_INFO("EnterableItem创建成功");
    item->bindParent(parent);
    item->id = _id;
    
    return *item;
}

EnterableItem& EnterableItem::setTittle(const char* tittle){
    this->tittle = tittle;
    return *this;
}

EnterableItem& EnterableItem::setChildShowType(ItemShowType type){
    child_show_type = type;
    return *this;
}

void EnterableItem::operate_enter(sLittleMenu& menu){
    if(child_show_type == ItemShowType::LIST){
        if(menu.curr->child){
            menu.curr->is_hover = false;
            menu.curr = menu.curr->child;
            menu.curr->is_hover = true;
        }else{
            SLM_LOG_WARN("ENTER操作失败,子菜单为空");
        }
    }
    else if(child_show_type == ItemShowType::CANVAS){
        if(canvas_enter_callback && menu.curr->is_hover){
            canvas_enter_callback(static_cast<EnterableItem*>(this),this->id);
        }
        menu.curr->is_hover = false;
    }
}

void EnterableItem::operate_back(sLittleMenu& menu){
    if(child_show_type == ItemShowType::LIST){
        //不能回到根节点
        if(menu.curr->parent->parent){
            menu.curr->is_hover = false;
            menu.curr = menu.curr->parent;
            menu.curr->is_hover = true;
        }else{
            SLM_LOG_WARN("BACK操作失败,父菜单为空");
        }
    }
    else if(child_show_type == ItemShowType::CANVAS){
        if(canvas_exit_callback && !menu.curr->is_hover){
            canvas_exit_callback(static_cast<EnterableItem*>(this),this->id);
        }
        menu.curr->is_hover = true;
    }
}

void EnterableItem::operate_prev(sLittleMenu& menu){
    if(menu.curr->prev_sibling){
        menu.curr->is_hover = false;
        menu.curr = menu.curr->prev_sibling;
        menu.curr->is_hover = true;
        
    }else{
        SLM_LOG_WARN("PREV操作失败,上级菜单为空");
    }
}

void EnterableItem::operate_next(sLittleMenu& menu){
    if(menu.curr->next_sibling){
        menu.curr->is_hover = false;
        menu.curr = menu.curr->next_sibling;
        menu.curr->is_hover = true;
    }else{
        SLM_LOG_WARN("NEXT操作失败,下级菜单为空");
    }
}





LabelItem& LabelItem::create(ItemBase* parent,uint32_t _id){
    LabelItem* item = SLM_CREATE_CLASS(LabelItem,);
    if(!item){
        SLM_LOG_ERR("malloc返回空指针,LabelItem创建失败");
    }
    SLM_LOG_INFO("LabelItem创建成功");
    item->bindParent(parent);
    item->id = _id;
    
    return *item;
}

LabelItem& LabelItem::setTittle(const char* tittle){
    this->tittle = tittle;
    return *this;
}

ButtonItem& ButtonItem::create(ItemBase* parent,uint32_t _id){
    ButtonItem* item = SLM_CREATE_CLASS(ButtonItem,);
    if(!item){
        SLM_LOG_ERR("malloc返回空指针,LabelItem创建失败");
    }
    SLM_LOG_INFO("LabelItem创建成功");
    item->bindParent(parent);
    item->id = _id;
    
    return *item;
}

ButtonItem& ButtonItem::setContext(const char* tittle,const char* cover_text){
    this->tittle = tittle;
    this->cover_text = cover_text;
    return *this;
}

ButtonItem& ButtonItem::setCallback(ButtonPressCb press_cb){
    press_callback = press_cb;
    return *this;
}





IntValAdj& IntValAdj::create(ItemBase* parent,uint32_t _id){
    IntValAdj* item = SLM_CREATE_CLASS(IntValAdj,);
    if(!item){
        SLM_LOG_ERR("malloc返回空指针,IntValAdj创建失败");
    }
    SLM_LOG_INFO("IntValAdj创建成功");
    item->bindParent(parent);
    item->id = _id;
    item->show_fmt = SLM_INT_VAL_ADJ_DEFAULT;
    return *item;
}

IntValAdj& IntValAdj::setContext(const char* tittle,const char* show_fmt,int default_val,int increment,int decrement){
    this->tittle = tittle;
    //如果fmt是空指针,则是默认值
    if(show_fmt){this->show_fmt = show_fmt;}
    value = default_val;
    this->increment = increment;
    this->decrement = decrement;
    return *this;
}

IntValAdj& IntValAdj::setCallback(IntValChangeCb change_cb,CallBackMethod cb_method){
    this->change_callback = change_cb;
    this->callback_method = cb_method;
    return *this;
}

IntValAdj& IntValAdj::setConstraint(ConstraintType cons_type,int max,int min){
    this->cons_t = cons_type;
    this->max = max;
    this->min = min;
    return *this;
}

void IntValAdj::operate_enter(sLittleMenu& menu){
    if(!is_selected){is_selected = true;}
}

void IntValAdj::operate_back(sLittleMenu& menu){
    //先解锁
    if(is_selected == true){
        if(callback_method == CallBackMethod::EXIT && change_callback != nullptr){
            change_callback(this,id,value);
        }
        is_selected = false;
    }else{
        //不能回到根节点
        if(menu.curr->parent->parent){
            menu.curr->is_hover = false;
            menu.curr = menu.curr->parent;
            menu.curr->is_hover = true;
        }else{
            SLM_LOG_WARN("BACK操作失败,父菜单为空");
        }
    }
}

void IntValAdj::operate_prev(sLittleMenu& menu){
    //没选中时执行prev操作
    if(!is_selected){
        if(menu.curr->prev_sibling){
            menu.curr->is_hover = false;
            menu.curr = menu.curr->prev_sibling;
            menu.curr->is_hover = true;
        }else{
            SLM_LOG_WARN("PREV操作失败,上级菜单为空");
        }
    }
    //选中时执行修改
    else{
        incDecProcess(true);
        if(callback_method == CallBackMethod::CHANGE && change_callback != nullptr){
            change_callback(this,id,value);
        }
        SLM_LOG_INFO("IntValAdj修改完成");
    }
}

void IntValAdj::operate_next(sLittleMenu& menu){
    if(!is_selected){
        if(menu.curr->next_sibling){
            menu.curr->is_hover = false;
            menu.curr = menu.curr->next_sibling;
            menu.curr->is_hover = true;
        }else{
            SLM_LOG_WARN("NEXT操作失败,下级菜单为空");
        }
    }
    else{
        incDecProcess(false);
        if(callback_method == CallBackMethod::CHANGE && change_callback != nullptr){
            change_callback(this,id,value);
        }
        SLM_LOG_INFO("IntValAdj修改完成");
    }
}

void IntValAdj::print() const{
    const char* text_ptr = getValText();
    SLM_PRINTF("id=%u,IntValueAdj:%s,val:%s,S=%d,H=%d\n",id,tittle,text_ptr,is_selected?1:0,is_hover?1:0);
}



const char* IntValAdj::getValText() const{
    snprintf((char*)value_text,SLM_INT_VAL_ADJ_VAL_LEN,show_fmt,value);
    return value_text;
}


void IntValAdj::incDecProcess(bool is_inc){
    //先加减再限幅
    is_inc ? value += increment : value -= decrement;
    //限幅处理
    if(cons_t == ConstraintType::MAX || cons_t == ConstraintType::RANGE)if(value > max)value = max;
    if(cons_t == ConstraintType::MIN || cons_t == ConstraintType::RANGE)if(value < min)value = min;
}



FloatValAdj& FloatValAdj::create(ItemBase* parent,uint32_t _id){
    FloatValAdj* item = SLM_CREATE_CLASS(FloatValAdj,);
    if(!item){
        SLM_LOG_ERR("malloc返回空指针,FloatValAdj创建失败");
    }
    SLM_LOG_INFO("FloatValAdj创建成功");
    item->bindParent(parent);
    item->id = _id;
    item->show_fmt = SLM_FLOAT_VAL_ADJ_DEFAULT;
    return *item;
}

FloatValAdj& FloatValAdj::setContext(const char* tittle,const char* show_fmt,float default_val,float increment,float decrement){
    this->tittle = tittle;
    //如果fmt是空指针,则是默认值
    if(show_fmt){this->show_fmt = show_fmt;}
    value = default_val;
    this->increment = increment;
    this->decrement = decrement;
    return *this;
}

FloatValAdj& FloatValAdj::setCallback(FloatValChangeCb change_cb,CallBackMethod cb_method){
    this->change_callback = change_cb;
    this->callback_method = cb_method;
    return *this;
}

FloatValAdj& FloatValAdj::setConstraint(ConstraintType cons_type,float max,float min){
    this->cons_t = cons_type;
    this->max = max;
    this->min = min;
    return *this;
}

void FloatValAdj::operate_enter(sLittleMenu& menu){
    if(!is_selected){is_selected = true;}
}

void FloatValAdj::operate_back(sLittleMenu& menu){
    //先解锁
    if(is_selected == true){
        if(callback_method == CallBackMethod::EXIT && change_callback != nullptr){
            change_callback(this,id,value);
        }
        is_selected = false;
    }else{
        //不能回到根节点
        if(menu.curr->parent->parent){
            menu.curr->is_hover = false;
            menu.curr = menu.curr->parent;
            menu.curr->is_hover = true;
        }else{
            SLM_LOG_WARN("BACK操作失败,父菜单为空");
        }
    }
}

void FloatValAdj::operate_prev(sLittleMenu& menu){
    //没选中时执行prev操作
    if(!is_selected){
        if(menu.curr->prev_sibling){
            menu.curr->is_hover = false;
            menu.curr = menu.curr->prev_sibling;
            menu.curr->is_hover = true;
        }else{
            SLM_LOG_WARN("PREV操作失败,上级菜单为空");
        }
    }
    //选中时执行修改
    else{
        incDecProcess(true);
        if(callback_method == CallBackMethod::CHANGE && change_callback != nullptr){
            change_callback(this,id,value);
        }
        SLM_LOG_INFO("IntValAdj修改完成");
    }
}

void FloatValAdj::operate_next(sLittleMenu& menu){
    if(!is_selected){
        if(menu.curr->next_sibling){
            menu.curr->is_hover = false;
            menu.curr = menu.curr->next_sibling;
            menu.curr->is_hover = true;
        }else{
            SLM_LOG_WARN("NEXT操作失败,下级菜单为空");
        }
    }
    else{
        incDecProcess(false);
        if(callback_method == CallBackMethod::CHANGE && change_callback != nullptr){
            change_callback(this,id,value);
        }
        SLM_LOG_INFO("IntValAdj修改完成");
    }
}

void FloatValAdj::print() const{
    const char* text_ptr = getValText();
    SLM_PRINTF("id=%u,FloatValueAdj:%s,val:%s,S=%d,H=%d\n",id,tittle,text_ptr,is_selected?1:0,is_hover?1:0);
}



const char* FloatValAdj::getValText() const{
    snprintf((char*)value_text,SLM_FLOAT_VAL_ADJ_VAL_LEN,show_fmt,value);
    return value_text;
}

void FloatValAdj::incDecProcess(bool is_inc){
    //先加减再限幅
    is_inc ? value += increment : value -= decrement;
    //限幅处理
    if(cons_t == ConstraintType::MAX || cons_t == ConstraintType::RANGE)if(value > max)value = max;
    if(cons_t == ConstraintType::MIN || cons_t == ConstraintType::RANGE)if(value < min)value = min;
}


SwitchItem& SwitchItem::create(uint32_t _id){
    SwitchItem* item = SLM_CREATE_CLASS(SwitchItem,);
    if(!item)SLM_LOG_ERR("malloc返回空指针,SwitchItem创建失败");
    item->id = _id;
    return *item;
}

SwitchItem& SwitchItem::create(ItemBase* parent,uint32_t _id){
    SwitchItem* item = SLM_CREATE_CLASS(SwitchItem,);
    if(!item)SLM_LOG_ERR("malloc返回空指针,SwitchItem创建失败");
    item->bindParent(parent);
    item->id = _id;
    return *item;
}

SwitchItem& SwitchItem::setContext(const char* tittle){
    this->tittle = tittle;
    return *this;
}

SwitchItem& SwitchItem::setContext(const char* tittle,const char* on_text,const char* off_text){
    this->tittle = tittle;
    if(on_text)this->on_text = on_text;
    if(off_text)this->off_text = off_text;
    return *this;
}

SwitchItem& SwitchItem::setCallback(SwitchPressCb press_cb){
    press_callback = press_cb;
    return *this;
}

SwitchItem& SwitchItem::setStatus(bool status){
    this->status = status;
    return *this;
}

IntValShow& IntValShow::create(ItemBase* parent,uint32_t _id){
    IntValShow* item = SLM_CREATE_CLASS(IntValShow,);
    if(!item){
        SLM_LOG_ERR("malloc返回空指针,IntValShow创建失败");
    }
    SLM_LOG_INFO("IntValShow创建成功");
    item->bindParent(parent);
    item->id = _id;
    item->show_fmt = SLM_INT_VAL_SHOW_DEFAULT;
    return *item;
}

IntValShow& IntValShow::setContext(const char* tittle,const char* show_fmt){
    this->tittle = tittle;
    //如果fmt是空指针,则是默认值
    if(show_fmt){this->show_fmt = show_fmt;}
    return *this;
}

IntValShow& IntValShow::setCallback(IntValGetCb get_cb){
    get_callback = get_cb;
    return *this;
}


void IntValShow::print() const{
    const char* text_ptr = const_cast<IntValShow*>(this)->getValText();
    SLM_PRINTF("id=%u,IntValueShow:%s,val:%s,H=%d\n",id,tittle,text_ptr,is_hover?1:0);
}


const char* IntValShow::getValText(){
    update_value();
    snprintf((char*)value_text,SLM_INT_VAL_SHOW_VAL_LEN,show_fmt,value);
    return value_text;
}

FloatValShow& FloatValShow::create(ItemBase* parent,uint32_t _id){
    FloatValShow* item = SLM_CREATE_CLASS(FloatValShow,);
    if(!item){
        SLM_LOG_ERR("malloc返回空指针,FloatValShow创建失败");
    }
    SLM_LOG_INFO("FloatValShow创建成功");
    item->bindParent(parent);
    item->id = _id;
    item->show_fmt = SLM_FLOAT_VAL_SHOW_DEFAULT;
    return *item;
}

FloatValShow& FloatValShow::setContext(const char* tittle,const char* show_fmt){
    this->tittle = tittle;
    //如果fmt是空指针,则是默认值
    if(show_fmt){this->show_fmt = show_fmt;}
    return *this;
}

FloatValShow& FloatValShow::setCallback(FloatValGetCb get_cb){
    get_callback = get_cb;
    return *this;
}


void FloatValShow::print() const{
    const char* text_ptr = const_cast<FloatValShow*>(this)->getValText();
    SLM_PRINTF("id=%u,FloatValueShow:%s,val:%s,H=%d\n",id,tittle,text_ptr,is_hover?1:0);
}


const char* FloatValShow::getValText(){
    update_value();
    snprintf((char*)value_text,SLM_FLOAT_VAL_SHOW_VAL_LEN,show_fmt,value);
    return value_text;
}


//todo 
StringValShow& StringValShow::create(ItemBase* parent,uint32_t _id){
    StringValShow* item = SLM_CREATE_CLASS(StringValShow,);
    if(!item){
        SLM_LOG_ERR("malloc返回空指针,StringValShow创建失败");
    }
    SLM_LOG_INFO("StringValShow创建成功");
    item->bindParent(parent);
    item->id = _id;
    return *item;
}

StringValShow& StringValShow::setContext(const char* tittle){
    this->value_text = tittle;
    return *this;
}

StringValShow& StringValShow::setCallback(StringValGetCb get_cb){
    get_callback = get_cb;
    return *this;
}

StringValShow& StringValShow::setTittle(const char* tittle){
    this->tittle = tittle;
    return *this;
}


void StringValShow::print() const{
    const char* text_ptr = const_cast<StringValShow*>(this)->getValText();
    SLM_PRINTF("id=%u,StringValShow:%s,val:%s,H=%d\n",id,tittle,text_ptr,is_hover?1:0);
}


const char* StringValShow::getValText(){
    update_value();
    return value_text;
}





}


