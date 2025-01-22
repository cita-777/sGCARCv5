#include "sLittleMenu.hpp"


#ifdef SLM_DEBUG_LOG_EN
    #define LOG_INFO(_TEXT) dbg_info("sLM line:%u,%s\n",__LINE__,_TEXT)
    #define LOG_WARN(_TEXT) dbg_warn("sLM line:%u,%s\n",__LINE__,_TEXT)
    #define LOG_ERR(_TEXT)  dbg_printf("[ERR ] sLM line:%u,%s\n",__LINE__,_TEXT)
#else
    #define LOG_INFO(_TEXT) (void)0
    #define LOG_WARN(_TEXT) (void)0
    #define LOG_ERR(_TEXT)  (void)0
#endif



sLM::sLittleMenu menu;




namespace sLM{




//虚析构,保证删除此节点时,会递归的删除所有子节点和兄弟节点
TreeNode::~TreeNode(){
    delete child;
    delete next_sibling;
}

//静态函数,用于创建一个TreeNode,并将src_data拷贝到data[](深拷贝)
TreeNode* TreeNode::createNode(const void* src_data, size_t dsize){
    //分配足够的内存 sizeof(TreeNode)包含了data[1]，需要额外(dsize-1)来补足所需空间
    void* mem = pvPortMalloc(sizeof(TreeNode) + dsize - 1);
    if(!mem){
        LOG_ERR("allocation failed");
        return nullptr;
    }
    //placement new! 在已分配的内存上构造对象
    TreeNode* node = new(mem) TreeNode(dsize);
    //拷贝数据
    if(src_data != nullptr && dsize > 0){
        memcpy(node->data, src_data, dsize);
    }else{
        LOG_ERR("src_data is nullptr or dsize is 0");
        return nullptr;
    }
    return node;
}

int TreeNode::addChild(TreeNode* newChild){
    if(!newChild) return -1;
        //你的爸爸是我
        newChild->parent = this;
        //如果当前的节点还没有任何子节点,说明这是第一个,就直接赋值
        if (!child){
            child = newChild;
        }
        //你已经不是第一个了
        else{
            TreeNode* temp = child;
            //找到最后一个节点(sibling为nullptr)
            while(temp->next_sibling)temp = temp->next_sibling;
            //连接上最后一个节点
            temp->next_sibling = newChild;
            //设置新子节点的上一个节点
            newChild->prev_sibling = temp;
        }
    return 0;
}

//调试用
void TreeNode::printTree(int level, void (*printFunc)(unsigned char*, size_t)){
        for (int i = 0; i < level; ++i) {
            sBSP_UART_Debug_Printf("    ");
        }
        printFunc(data, data_size);
        if (child) {
            child->printTree(level + 1, printFunc);
        }
        if (next_sibling) {
            next_sibling->printTree(level, printFunc);
        }
    }




sLittleMenu::sLittleMenu(){

}

sLittleMenu::~sLittleMenu(){

}


//初始化菜单
void sLittleMenu::init(Renderer* _renderer){
    //创建一个根节点
    ItemData root_item = {.id = 0,.text = SLM_ROOT_MENU_TEXT,\
                            .child_show_mode = sLM::MenuChildShowMode::LIST};
    root = TreeNode::createNode((const void*)&root_item,sizeof(root_item));
    curr = root;
    id_count++;
    if(id_count >= SLM_MAX_ITEM_NUM){
        LOG_ERR("已达最大的菜单项数量");
    }

    //记录到qlist
    qlist[0] = root;

    renderer = _renderer;

}

//添加一个子节点
int sLittleMenu::addSubMenu(TreeNode* parent,TreeNode* child){
    if(!parent && !child) return -1;    
    //如果这是第一个节点
    if(!root->child){
        curr = child;   //进入他,防止在root节点
        ItemData& data = getNodeData(curr);
        data.is_hover = true; //初始选中
    }
    ItemData& node = getNodeData(child);
    node.id = id_count;
    //记录到qlist
    qlist[id_count] = child;
    id_count++;
    //添加到父节点的子节点列表
    return parent->addChild(child);
}

//创建一个item数据
void sLittleMenu::setItemData(ItemData* item_data,ItemDataCreateItemConf* config){
    memset(item_data,0,sizeof(ItemData));
    strcpy(item_data->text,config->text);
    item_data->param.access = config->access;
    item_data->param.type = config->param_type;
    item_data->type = config->item_type;
    item_data->param.modify_cb = config->change_cb;
    item_data->param.modify_cb_method = config->change_method;
    item_data->param.lim_type = config->limit_type;
    item_data->param.update_cb = config->update_cb;
    item_data->param.param_tag = config->param_tag;
}

//获取同级的第一个节点
TreeNode& sLittleMenu::getFristNode(){
    TreeNode* node = curr; //从当前节点开始
    //沿着prev_sibling向上遍历直到找到最顶层节点
    while(node->prev_sibling)node = node->prev_sibling;
    return *node; //返回最上层节点
}

//使用任意兄弟节点获取同级的第一个节点
TreeNode& sLittleMenu::getFristNode(TreeNode* any_sibling){
    TreeNode* node = any_sibling;
    //沿着prev_sibling向上遍历直到找到最顶层节点
    while (node->prev_sibling)node = node->prev_sibling;
    return *node; //返回最上层节点
}

//获取当前节点的同级的索引值
uint32_t sLittleMenu::getCurrMenuIndex(){
    uint32_t index = 1;
    TreeNode* node = curr;
    while(node->prev_sibling){
        node = node->prev_sibling;
        index++;
    }
    return index;
}

//获取当前菜单项同级有多少个菜单
uint32_t sLittleMenu::getCurrMenuNumber(){
    uint32_t count = 0;
    TreeNode* node = &getFristNode();
    while(node){
        node = node->next_sibling;
        count++;
    }
    return count;
}

//通过索引返回当前同级的菜单项
TreeNode* sLittleMenu::getIndexMenu(uint32_t index){
    TreeNode* node = &getFristNode();
    while(index-- > 1){
        node = node->next_sibling;
    }
    return node;
}

//返回任意兄弟菜单的索引
TreeNode* sLittleMenu::getIndexMenu(TreeNode* any_slibing ,uint32_t index){
    //找到最上面的节点
    TreeNode* node = &getFristNode(any_slibing);
    while(index-- > 1){
        node = node->next_sibling;
    }
    //再通过索引找到需要的菜单项
    node = getIndexMenu(index);
    return node;
}

//参数增量处理
void sLittleMenu::incDecParamHandler(ItemData& item,ParamModifyDir dir){
    //获取item的param段
    Param& param = item.param;
    if(param.type == ParamType::STRING)return;    //不处理字符串类型,留给用户处理
    //获取item的限幅类型
    ParamLimitType lim_t = param.lim_type;

    //参数值为int
    if(param.type == sLM::ParamType::INT){
        int& val = param.val_i;
        int& max = param.max_i; int& min = param.min_i;
        int& inc = param.inc_i; int& dec = param.dec_i;
        //先加减再限幅
        dir == ParamModifyDir::UP ? val += inc : val -= dec;
        //限幅处理
        if(lim_t == ParamLimitType::MAX || lim_t == ParamLimitType::RANGE)if(val > max)val = max;
        if(lim_t == ParamLimitType::MIN || lim_t == ParamLimitType::RANGE)if(val < min)val = min;
    }
    //float
    else if(param.type == sLM::ParamType::FLOAT){
        float& val = param.val_f;
        float& max = param.max_f; float& min = param.min_f;
        float& inc = param.inc_f; float& dec = param.dec_f;
        //先加减再限幅
        dir == ParamModifyDir::UP ? val += inc : val -= dec;
        //限幅处理
        if(lim_t == ParamLimitType::MAX || lim_t == ParamLimitType::RANGE)if(val > max)val = max;
        if(lim_t == ParamLimitType::MIN || lim_t == ParamLimitType::RANGE)if(val < min)val = min;
    }
}

void sLittleMenu::setLock(const char* tittle,const char* msg){
    lock_info.status = true;
    strncpy(lock_info.tittle,tittle,SLM_LOCK_TITTLE_LEN);
    strncpy(lock_info.message,msg,SLM_LOCK_MESSAGE_LEN);
}

void sLittleMenu::setLock(){
    lock_info.status = true;
    strncpy(lock_info.tittle,SLM_LOCK_DEFAULT_TITTLE,SLM_LOCK_TITTLE_LEN);
    strncpy(lock_info.message,SLM_LOCK_DEFAULT_MESSAGE,SLM_LOCK_MESSAGE_LEN);
}

void sLittleMenu::setUnlock(){
    lock_info.status = false;
}



void sLittleMenu::operateEnter(){
    if(op_event == OpEvent::NONE){
        op_event = OpEvent::ENTER;
    }else{
        LOG_WARN("发生了Enter事件,但是当前的操作事件不为空");
    }
}

void sLittleMenu::operateBack(){
    if(op_event == OpEvent::NONE){
        op_event = OpEvent::BACK;
    }else{
        LOG_WARN("发生了Back事件,但是当前的操作事件不为空");
    }
}

void sLittleMenu::operatePrev(){
    if(op_event == OpEvent::NONE){
        op_event = OpEvent::PREV;
    }else{
        LOG_WARN("发生了Prev事件,但是当前的操作事件不为空");
    }
}

void sLittleMenu::operateNext(){
    if(op_event == OpEvent::NONE){
        op_event = OpEvent::NEXT;
    }else{
        LOG_WARN("发生了Next事件,但是当前的操作事件不为空");
    }
}

//Enter操作
void sLittleMenu::op_enter_process(ItemData& curr_item){
    //如果当前页面有子页面
    if(curr->child){
        //让旧的节点取消hover
        curr_item.is_hover = false;
        //跳转到子页面
        curr = curr->child;
        //让新的节点hover
        getCurrNodeData().is_hover = true;
    }
    //没有下一级了,说明要选中
    else{
        if(curr_item.type == ItemType::NORMAL){
            //参数可读写时才能选中
            if(curr_item.param.access == ParamAccess::RW)curr_item.is_selected = true;
        }
        //如果item是一个按键,则调用用户注册的回调
        else if(curr_item.type == ItemType::BUTTON){
            if(curr_item.param.modify_cb){
                //void* param传的是当前的参数的标签
                lock_info.status = (bool)curr_item.param.modify_cb(&curr_item.param.param_tag,ParamType::BUTTON_PRESS);
            }
        }
        
    }
}

void sLittleMenu::op_back_process(ItemData& curr_item){
    //如果当前项被选中了,就退出选中
    if(curr_item.is_selected == true){
        curr_item.is_selected = false;
    }
    //如果没有被选中,并且不是root则跳转到上一级
    else if(curr->parent != root){
        //让旧的节点取消hover
        curr_item.is_hover = false;
        curr = curr->parent;
        //让新的节点hover
        getCurrNodeData().is_hover = true;
    }

    //如果配置的是退出回调,参数被修改了调用用户的函数
    if(curr_item.param.is_param_chrg && curr_item.param.modify_cb_method == ParamModifyCbMethod::EXIT_CB){
        //调用用户函数
        if(curr_item.param.modify_cb != nullptr){
            lock_info.status = (bool)curr_item.param.modify_cb(&curr_item.param.val_i,curr_item.param.type);
        }
        curr_item.param.is_param_chrg = false;
    }
}

void sLittleMenu::op_prev_process(ItemData& curr_item){
    //如果已经被选中了就修改参数
    if(curr_item.is_selected == true && lock_info.status == false){
        incDecParamHandler(curr_item, ParamModifyDir::UP);
        curr_item.param.is_param_chrg = true;
    }
    //没有选中就上下移动
    else if(curr->prev_sibling){
        //让旧的节点取消hover
        curr_item.is_hover = false;
        curr = curr->prev_sibling;
        //让新的节点hover
        getNodeData(curr).is_hover = true;
    }

    //如果配置的是修改就回调,参数被修改了调用用户的函数
    if(curr_item.param.is_param_chrg && curr_item.param.modify_cb_method == ParamModifyCbMethod::CHRG_CB){  
        //调用用户函数
        if(curr_item.param.modify_cb != nullptr){
            lock_info.status = (bool)curr_item.param.modify_cb(&curr_item.param.val_i,curr_item.param.type);
        }
        curr_item.param.is_param_chrg = false;
    }
}

void sLittleMenu::op_next_process(ItemData& curr_item){
    //如果已经被选中并且已解锁就修改参数
    if(curr_item.is_selected == true && lock_info.status == false){
        incDecParamHandler(curr_item, ParamModifyDir::DN);
        curr_item.param.is_param_chrg = true;
    }
    //没有选中就上下移动
    else if(curr->next_sibling){
        //让旧的节点取消hover
        curr_item.is_hover = false;
        curr = curr->next_sibling;
        //让新的节点hover
        getNodeData(curr).is_hover = true;
    }

    //如果配置的是修改就回调,参数被修改了调用用户的函数
    if(curr_item.param.is_param_chrg && curr_item.param.modify_cb_method == ParamModifyCbMethod::CHRG_CB){
        //调用用户函数
        if(curr_item.param.modify_cb != nullptr){
            lock_info.status = (bool)curr_item.param.modify_cb(&curr_item.param.val_i,curr_item.param.type);
        }
        curr_item.param.is_param_chrg = false;
    }
}

void sLittleMenu::operate_process(){
    if(op_event == OpEvent::NONE)return;

    if(lock_info.status == true){
        op_event = OpEvent::NONE;   //避免重复触发
        LOG_INFO("操作无效,因为菜单被锁定");
        return;
    }

    ItemData& curr_item = getNodeData(curr);
    
    if(op_event == OpEvent::ENTER){
        op_enter_process(curr_item);
    }
    else if(op_event == OpEvent::BACK){
        op_back_process(curr_item);
    }
    else if(op_event == OpEvent::PREV){
        op_prev_process(curr_item);
    }
    else if(op_event == OpEvent::NEXT){
        op_next_process(curr_item);
    }

    //重置操作事件
    op_event = OpEvent::NONE;
}

uint16_t sLittleMenu::getItemCount(){
    return id_count;
}

TreeNode* sLittleMenu::getItemByID(uint16_t id){
    return qlist[id] ? qlist[id] : nullptr;
}

//更新
void sLittleMenu::update(){
    //首先处理输入操作
    operate_process();

    


    // if(getNodeData(curr).periodic_cb){
    //     getNodeData(curr).periodic_cb(curr);
    // }
    renderer->update();
}


// 自定义打印函数，用于打印ItemData
void printItemData(unsigned char* data, size_t data_size) {
    sLM::ItemData* item = reinterpret_cast<sLM::ItemData*>(data);
    sBSP_UART_Debug_Printf("- ID: %u,text:%s",item->id,item->text);

    if(item->param.type == sLM::ParamType::STRING){
        sBSP_UART_Debug_Printf(",str: %s ",item->param.val_s);
    }
    else if(item->param.type == sLM::ParamType::FLOAT){
        void* value_ptr  = &(item->param.val_f);
        float& value  = *(float*)value_ptr;
        sBSP_UART_Debug_Printf(",float: %.2f ",value);
    }
    else if(item->param.type == sLM::ParamType::INT){
        void* value_ptr  = &(item->param.val_i);
        int& value  = *(int*)value_ptr;
        sBSP_UART_Debug_Printf(",int: %d ",value);
    }
    sBSP_UART_Debug_Printf(", selected:%u,is_hover:%u\n",item->is_selected,item->is_hover);
}












}


