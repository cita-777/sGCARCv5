#include "sLittleMenu.hpp"


/**
 * sLittleMenu.cpp
 * Sightseer's LittleMenu 一个轻量级的菜单~
 * 实现了一个轻量级的菜单,此文件只负责菜单的结构管理,不负责显示,以此实现高通用性
 * 
 * v1.0 241211Night BySightseer.inHNIP9607
 * 第一版完工! demo做好了
 * 下一版改进:
 * todo string参数模式,用户自定义参数增量回调
 * todo 完成按钮bool类型
 * todo 小图标支持
 * 
 * 
 * 
 */


sLittleMenu slm;


sLittleMenu::sLittleMenu(){

}

sLittleMenu::~sLittleMenu(){

}




//初始化菜单
void sLittleMenu::init(){
    //创建一个根节点
    MenuItemData root_item = {.id = 0,.text = SLM_ROOT_MENU_TEXT,\
                            .child_show_mode = sLM::MenuChildShowMode::LIST};
    root = sLM_TreeNode::createNode((const void*)&root_item,sizeof(root_item));
    curr_page = root;
    id_count++;
}

//添加一个子节点
int sLittleMenu::addSubMenu(sLM_TreeNode* parent,sLM_TreeNode* child){
    if(!parent && !child) return -1;    
    //如果这是第一个节点
    if(!root->child){
        curr_page = child;
        sLM::MenuItemData& data = getNodeData(curr_page);
        data.is_hover = true; //初始选中
    }
    sLM::MenuItemData& node = getNodeData(child);
    node.id = id_count;
    id_count++;
    //添加到父节点的子节点列表
    return parent->addChild(child);
}

//创建一个item数据
void sLittleMenu::setItemData(MenuItemData* item_data,ItemDataCreateConf* config){
    memset(item_data,0,sizeof(MenuItemData));
    strcpy(item_data->text,config->text);
    item_data->param.access = config->access;
    item_data->param.type = config->param_type;
    item_data->type = config->item_type;
    item_data->param.cb_func = config->change_cb;
    item_data->param.cb_mthd = config->change_method;
    item_data->param.lim_type = config->limit_type;
    item_data->param.update_cb = config->update_cb;
    item_data->param.param_tag = config->param_tag;
}


//获取当前节点数据
sLM::MenuItemData& sLittleMenu::getNodeData(){
    sLM::MenuItemData* item = reinterpret_cast<sLM::MenuItemData*>(curr_page->data);
    return *item;
}

//获取任意节点数据
sLM::MenuItemData& sLittleMenu::getNodeData(sLM_TreeNode* node){
    sLM::MenuItemData* item = reinterpret_cast<sLM::MenuItemData*>(node->data);
    return *item;
}

//获取同级的第一个节点
sLM_TreeNode& sLittleMenu::getFristNode(){
    sLM_TreeNode* node = curr_page; //从当前节点开始
    //沿着prev_sibling向上遍历直到找到最顶层节点
    while(node->prev_sibling)node = node->prev_sibling;
    return *node; //返回最上层节点
}

//使用任意兄弟节点获取同级的第一个节点
sLM_TreeNode& sLittleMenu::getFristNode(sLM_TreeNode* any_sibling){
    sLM_TreeNode* node = any_sibling;
    //沿着prev_sibling向上遍历直到找到最顶层节点
    while (node->prev_sibling)node = node->prev_sibling;
    return *node; //返回最上层节点
}

//获取当前节点的同级的索引值
uint32_t sLittleMenu::getCurrMenuIndex(){
    uint32_t index = 1;
    sLM_TreeNode* node = curr_page;
    while(node->prev_sibling){
        node = node->prev_sibling;
        index++;
    }
    return index;
}

//获取当前菜单项同级有多少个菜单
uint32_t sLittleMenu::getCurrMenuNumber(){
    uint32_t count = 0;
    sLM_TreeNode* node = &getFristNode();
    while(node){
        node = node->next_sibling;
        count++;
    }
    return count;
}

//通过索引返回当前同级的菜单项
sLM_TreeNode* sLittleMenu::getIndexMenu(uint32_t index){
    sLM_TreeNode* node = &getFristNode();
    while(index-- > 1){
        node = node->next_sibling;
    }
    return node;
}

//返回任意兄弟菜单的索引
sLM_TreeNode* sLittleMenu::getIndexMenu(sLM_TreeNode* any_slibing ,uint32_t index){
    //找到最上面的节点
    sLM_TreeNode* node = &getFristNode(any_slibing);
    while(index-- > 1){
        node = node->next_sibling;
    }
    //再通过索引找到需要的菜单项
    node = getIndexMenu(index);
    return node;
}

//返回根节点
void sLittleMenu::reset(){
    curr_page = root->child;
}

//参数增量了处理
void sLittleMenu::incDecParamHandler(MenuItemData& item,Item_ParamChrgDir dir){
    if(item.param.type == Item_ParamType::STRING)return;    //不处理字符串类型
    /*指向各参数值*/
    void* value_ptr  = &(item.param.value);
    void* top_ptr    = &(item.param.max);
    void* bottom_ptr = &(item.param.min);
    void* inc_ptr    = &(item.param.increment);
    void* dec_ptr    = &(item.param.decrement);
    
    //参数值为int
    if(item.param.type == sLM::Item_ParamType::INT){
        int& value  = *(int*)value_ptr;
        int& top    = *(int*)top_ptr; int& bottom = *(int*)bottom_ptr;
        int& inc    = *(int*)inc_ptr; int& dec    = *(int*)dec_ptr;
        //先加减再限幅
        dir == Item_ParamChrgDir::UP ? value += inc : value -= dec;
        //限幅处理
        if(item.param.lim_type == Item_ParamLimitType::MAX || item.param.lim_type == Item_ParamLimitType::RANGE){
            //向上限幅
            if(value > top)value = top;
        }
        if(item.param.lim_type == Item_ParamLimitType::MIN || item.param.lim_type == Item_ParamLimitType::RANGE){
            //向下限幅
            if(value < bottom)value = bottom;
        }
    }
    //float
    else if(item.param.type == sLM::Item_ParamType::FLOAT){
        float& value  = *(float*)value_ptr;
        float& top    = *(float*)top_ptr; float& bottom = *(float*)bottom_ptr;
        float& inc    = *(float*)inc_ptr; float& dec    = *(float*)dec_ptr;
        //先加减再限幅
        dir == Item_ParamChrgDir::UP ? value += inc : value -= dec;
        //限幅处理
        if(item.param.lim_type == Item_ParamLimitType::MAX || item.param.lim_type == Item_ParamLimitType::RANGE){
            //向上限幅
            if(value > top)value = top;
        }
        if(item.param.lim_type == Item_ParamLimitType::MIN || item.param.lim_type == Item_ParamLimitType::RANGE){
            //向下限幅
            if(value < bottom)value = bottom;
        }
    }
}

//设置item数据,int
void sLittleMenu::setItemData(MenuItemData* item_data,const char* _text,const char* _unit,int _value,int _min,int _max,int _inc,int _dec){
    /*指向各参数值*/
    void* value_ptr  = &(item_data->param.value);
    void* max_ptr    = &(item_data->param.max);       void* min_ptr    = &(item_data->param.min);
    void* inc_ptr    = &(item_data->param.increment); void* dec_ptr    = &(item_data->param.decrement);
    void* text_ptr   = &(item_data->text);            void* unit_ptr   = &(item_data->param.unit);

    int&  value = *(int*) value_ptr;
    int&  max   = *(int*) max_ptr;   int&  min  = *(int*) min_ptr;
    int&  inc   = *(int*) inc_ptr;   int&  dec  = *(int*) dec_ptr;
    char* text  =  (char*)text_ptr; char* unit =  (char*)unit_ptr;

    value = _value;
    max = _max; min = _min;
    inc = _inc; dec = _dec;
    strcpy(text,_text);
    strcpy(unit,_unit);
}

//设置item数据,float
void sLittleMenu::setItemData(MenuItemData* item_data,const char* _text,const char* _unit,float _value,float _min,float _max,float _inc,float _dec){
    /*指向各参数值*/
    void* value_ptr  = &(item_data->param.value);
    void* max_ptr    = &(item_data->param.max);       void* min_ptr    = &(item_data->param.min);
    void* inc_ptr    = &(item_data->param.increment); void* dec_ptr    = &(item_data->param.decrement);
    void* text_ptr   = &(item_data->text);            void* unit_ptr   = &(item_data->param.unit);

    float&  value = *(float*) value_ptr;
    float&  max   = *(float*) max_ptr;   float&  min  = *(float*) min_ptr;
    float&  inc   = *(float*) inc_ptr;   float&  dec  = *(float*) dec_ptr;
    char*   text  =  (char*)text_ptr;    char*   unit =  (char*)unit_ptr;

    value = _value;
    max = _max; min = _min;
    inc = _inc; dec = _dec;
    strcpy(text,_text);
    strcpy(unit,_unit);
}

//Enter操作
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
        if(item.type == ItemType::NORMAL){
            //参数可读写时才能选中
            if(item.param.access == Item_ParamAccess::RW)item.is_selected = true;
        }
        //如果item是一个按键,则调用用户注册的回调
        else if(item.type == ItemType::BUTTON){
            if(item.param.cb_func){
                //void* param传的是当前的参数的标签
                item.param.cb_func(&item.param.param_tag,Item_ParamType::BUTTON_PRESS);
            }
        }
        
    }

    //调用变化回调
    changeHandler(OpEvent::ENTER);
}

void sLittleMenu::opBack(){
    sLM::MenuItemData& item = getNodeData(curr_page);
    //如果当前项被选中了,就退出选中
    if(item.is_selected == true){
        item.is_selected = false;
    }
    //如果没有被选中,并且不是root则跳转到上一级
    else if(curr_page->parent != root){
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
    if(item.is_selected == true && item.param.status == Item_ParamCbRetType::UNLOCK){
        incDecParamHandler(item, Item_ParamChrgDir::UP);
        item.param.is_param_chrg = true;
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
    //如果已经被选中并且已解锁就修改参数
    if(item.is_selected == true && item.param.status == Item_ParamCbRetType::UNLOCK){
        incDecParamHandler(item, Item_ParamChrgDir::DN);
        item.param.is_param_chrg = true;
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

//有操作了的处理函数
void sLittleMenu::changeHandler(OpEvent ev){
    sLM::MenuItemData& curr_item = getNodeData();
    if(ev == OpEvent::ENTER){

    }
    else if(ev == OpEvent::BACK){
        if(curr_item.param.is_param_chrg && curr_item.param.cb_mthd == Item_ParamCbMethod::EXIT_CB){
            //调用用户函数
            curr_item.param.status = curr_item.param.cb_func(&curr_item.param.value,curr_item.param.type);
            curr_item.param.is_param_chrg = false;
        }
    }
    else if(ev == OpEvent::PREV){
        if(curr_item.param.is_param_chrg && curr_item.param.cb_mthd == Item_ParamCbMethod::CHRG_CB){
            //调用用户函数
            curr_item.param.status = curr_item.param.cb_func(&curr_item.param.value,curr_item.param.type);
            curr_item.param.is_param_chrg = false;
        }
    }
    else if(ev == OpEvent::NEXT){
        if(curr_item.param.is_param_chrg && curr_item.param.cb_mthd == Item_ParamCbMethod::CHRG_CB){
            //调用用户函数
            curr_item.param.status = curr_item.param.cb_func(&curr_item.param.value,curr_item.param.type);
            curr_item.param.is_param_chrg = false;
        }
    }


}

void sLittleMenu::update(){

}


//示例参数回调
sLittleMenu::Item_ParamCbRetType sLittleMenu::param_change(void* param,Item_ParamType type){
    uint32_t val = *(uint32_t*)param;
    if(type == Item_ParamType::INT){
        sBSP_UART_Debug_Printf("%d\n",val);
    }else{
        sBSP_UART_Debug_Printf("%.2f\n",val);
    }
    return Item_ParamCbRetType::UNLOCK;
}


// 自定义打印函数，用于打印MenuItemData
void printMenuItemData(unsigned char* data, size_t data_size) {
    sLM::MenuItemData* item = reinterpret_cast<sLM::MenuItemData*>(data);
    sBSP_UART_Debug_Printf("- ID: %u,text:%s",item->id,item->text);

    if(item->param.type == sLM::Item_ParamType::STRING){
        sBSP_UART_Debug_Printf(",str: %s ",item->param.value);
    }
    else if(item->param.type == sLM::Item_ParamType::FLOAT){
        void* value_ptr  = &(item->param.value);
        float& value  = *(float*)value_ptr;
        sBSP_UART_Debug_Printf(",float: %.2f ",value);
    }
    else if(item->param.type == sLM::Item_ParamType::INT){
        void* value_ptr  = &(item->param.value);
        int& value  = *(int*)value_ptr;
        sBSP_UART_Debug_Printf(",int: %d ",value);
    }
    sBSP_UART_Debug_Printf(", selected:%u,is_hover:%u\n",item->is_selected,item->is_hover);
}





