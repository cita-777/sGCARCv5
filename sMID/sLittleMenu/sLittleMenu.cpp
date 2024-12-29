#include "sLittleMenu.hpp"



#define LOG_INFO(_TEXT) sBSP_UART_Debug_Printf("[INFO] sLM file:%s,line:%u,%s\n",__FILE__,__LINE__,_TEXT)
#define LOG_WARN(_TEXT) sBSP_UART_Debug_Printf("[WARN] sLM file:%s,line:%u,%s\n",__FILE__,__LINE__,_TEXT)
#define LOG_ERR(_TEXT)  sBSP_UART_Debug_Printf("[ERR ] sLM file:%s,line:%u,%s\n",__FILE__,__LINE__,_TEXT)

// #define LOG_INFO(_LINE,_TEXT) (void)0
// #define LOG_WARN(_LINE,_TEXT) (void)0
// #define LOG_ERR(_LINE,_TEXT)  (void)0

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

    renderer = _renderer;

}

TreeNode* sLittleMenu::getRoot(){
    return root;
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
    item_data->param.modify_cb_mthd = config->change_method;
    item_data->param.lim_type = config->limit_type;
    item_data->param.update_cb = config->update_cb;
    item_data->param.param_tag = config->param_tag;
}



sLM::TreeNode* sLittleMenu::getCurr(){
    return curr;
}


//获取当前节点数据
sLM::ItemData& sLittleMenu::getCurrNodeData(){
    sLM::ItemData* item = reinterpret_cast<sLM::ItemData*>(curr->data);
    return *item;
}



//获取任意节点数据
sLM::ItemData& sLittleMenu::getNodeData(TreeNode* node){
    sLM::ItemData* item = reinterpret_cast<sLM::ItemData*>(node->data);
    return *item;
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

//返回根节点
void sLittleMenu::reset(){
    curr = root->child;
}

//参数增量处理
void sLittleMenu::incDecParamHandler(ItemData& item,ParamModifyDir dir){
    if(item.param.type == ParamType::STRING)return;    //不处理字符串类型,留给用户处理
    
    //参数值为int
    if(item.param.type == sLM::ParamType::INT){
        int& val = *&(item.param.val_i);
        int& max = *&(item.param.max_i); int& min = *&(item.param.min_i);
        int& inc = *&(item.param.inc_i); int& dec = *&(item.param.dec_i);
        //先加减再限幅
        dir == ParamModifyDir::UP ? val += inc : val -= dec;
        //限幅处理
        if(item.param.lim_type == ParamLimitType::MAX || item.param.lim_type == ParamLimitType::RANGE){
            //向上限幅
            if(val > max)val = max;
        }
        if(item.param.lim_type == ParamLimitType::MIN || item.param.lim_type == ParamLimitType::RANGE){
            //向下限幅
            if(val < min)val = min;
        }
    }
    //float
    else if(item.param.type == sLM::ParamType::FLOAT){
        float& val = *&(item.param.val_f);
        float& max = *&(item.param.max_f); float& min = *&(item.param.min_f);
        float& inc = *&(item.param.inc_f); float& dec = *&(item.param.dec_f);
        //先加减再限幅
        dir == ParamModifyDir::UP ? val += inc : val -= dec;
        //限幅处理
        if(item.param.lim_type == ParamLimitType::MAX || item.param.lim_type == ParamLimitType::RANGE){
            //向上限幅
            if(val > max)val = max;
        }
        if(item.param.lim_type == ParamLimitType::MIN || item.param.lim_type == ParamLimitType::RANGE){
            //向下限幅
            if(val < min)val = min;
        }
    }
}


void sLittleMenu::setLockCurrItem(bool is_lock){
    sLM::ItemData& item = getNodeData(curr);
    if(is_lock == true){
        item.param.lock = ParamModifyLock::LOCK;
    }else{
        item.param.lock = ParamModifyLock::UNLOCK;
    }
}


//Enter操作
void sLittleMenu::opEnter(){
    sLM::ItemData& item = getNodeData(curr);

    //操作时预先检查
    if(item.param.lock == ParamModifyLock::LOCK){
        changeHandler(OpEvent::LOCK);
        return;
    }

    //如果当前页面有子页面
    if(curr->child){
        //让旧的节点取消hover
        item.is_hover = false;
        //跳转到子页面
        curr = curr->child;
        //让新的节点hover
        getCurrNodeData().is_hover = true;
    }
    //没有下一级了,说明要选中
    else{
        if(item.type == ItemType::NORMAL){
            //参数可读写时才能选中
            if(item.param.access == ParamAccess::RW)item.is_selected = true;
        }
        //如果item是一个按键,则调用用户注册的回调
        else if(item.type == ItemType::BUTTON){
            if(item.param.modify_cb){
                //void* param传的是当前的参数的标签
                item.param.lock = item.param.modify_cb(&item.param.param_tag,ParamType::BUTTON_PRESS);
            }
        }
        
    }

    //调用变化回调
    changeHandler(OpEvent::ENTER);
}

void sLittleMenu::opBack(){
    sLM::ItemData& item = getNodeData(curr);

    //操作时预先检查
    if(item.param.lock == ParamModifyLock::LOCK){
        changeHandler(OpEvent::LOCK);
        return;
    }


    //如果当前项被选中了,就退出选中
    if(item.is_selected == true){
        item.is_selected = false;
    }
    //如果没有被选中,并且不是root则跳转到上一级
    else if(curr->parent != root){
        //让旧的节点取消hover
        item.is_hover = false;
        curr = curr->parent;
        //让新的节点hover
        getCurrNodeData().is_hover = true;
    }
    changeHandler(OpEvent::BACK);
}

void sLittleMenu::opPrev(){
    sLM::ItemData& item = getNodeData(curr);

    //操作时预先检查
    if(item.param.lock == ParamModifyLock::LOCK){
        changeHandler(OpEvent::LOCK);
        return;
    }


    //如果已经被选中了就修改参数
    if(item.is_selected == true && item.param.lock == ParamModifyLock::UNLOCK){
        incDecParamHandler(item, ParamModifyDir::UP);
        item.param.is_param_chrg = true;
    }
    //没有选中就上下移动
    else if(curr->prev_sibling){
        //让旧的节点取消hover
        item.is_hover = false;
        curr = curr->prev_sibling;
        //让新的节点hover
        getNodeData(curr).is_hover = true;
    }
    
    changeHandler(OpEvent::PREV);
}

void sLittleMenu::opNext(){
    sLM::ItemData& item = getNodeData(curr);

    //操作时预先检查
    if(item.param.lock == ParamModifyLock::LOCK){
        changeHandler(OpEvent::LOCK);
        return;
    }


    //如果已经被选中并且已解锁就修改参数
    if(item.is_selected == true && item.param.lock == ParamModifyLock::UNLOCK){
        incDecParamHandler(item, ParamModifyDir::DN);
        item.param.is_param_chrg = true;
    }
    //没有选中就上下移动
    else if(curr->next_sibling){
        //让旧的节点取消hover
        item.is_hover = false;
        curr = curr->next_sibling;
        //让新的节点hover
        getNodeData(curr).is_hover = true;
    }

    changeHandler(OpEvent::NEXT);
}

//有操作了的处理函数
void sLittleMenu::changeHandler(OpEvent ev){
    sLM::ItemData& curr_item = getCurrNodeData();

    if(ev == OpEvent::LOCK){

    }
    else if(ev == OpEvent::ENTER){

    }
    else if(ev == OpEvent::BACK){
        if(curr_item.param.is_param_chrg && curr_item.param.modify_cb_mthd == ParamModifyCbMethod::EXIT_CB){
            //调用用户函数
            if(curr_item.param.modify_cb != nullptr){
                curr_item.param.lock = curr_item.param.modify_cb(&curr_item.param.val_i,curr_item.param.type);
            }
            curr_item.param.is_param_chrg = false;
        }
    }
    else if(ev == OpEvent::PREV){
        if(curr_item.param.is_param_chrg && curr_item.param.modify_cb_mthd == ParamModifyCbMethod::CHRG_CB){
            //调用用户函数
            if(curr_item.param.modify_cb != nullptr){
                curr_item.param.lock = curr_item.param.modify_cb(&curr_item.param.val_i,curr_item.param.type);
            }
            curr_item.param.is_param_chrg = false;
        }
    }
    else if(ev == OpEvent::NEXT){
        if(curr_item.param.is_param_chrg && curr_item.param.modify_cb_mthd == ParamModifyCbMethod::CHRG_CB){
            //调用用户函数
            if(curr_item.param.modify_cb != nullptr){
                curr_item.param.lock = curr_item.param.modify_cb(&curr_item.param.val_i,curr_item.param.type);
            }
            curr_item.param.is_param_chrg = false;
        }
    }


}

void sLittleMenu::update(){
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


