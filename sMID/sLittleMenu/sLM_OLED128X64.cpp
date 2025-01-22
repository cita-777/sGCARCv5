#include "sLM_OLED128X64.hpp"



#ifdef SLM_DEBUG_LOG_EN
    #define LOG_INFO(_TEXT) dbg_info("sLM file:%s,line:%u,%s\n",__FILE__,__LINE__,_TEXT)
    #define LOG_WARN(_TEXT) dbg_warn("sLM file:%s,line:%u,%s\n",__FILE__,__LINE__,_TEXT)
    #define LOG_ERR(_TEXT)  dbg_printf("[ERR ] sLM file:%s,line:%u,%s\n",__FILE__,__LINE__,_TEXT)
#else
    #define LOG_INFO(_TEXT) (void)0
    #define LOG_WARN(_TEXT) (void)0
    #define LOG_ERR(_TEXT)  (void)0
#endif




using namespace sLM;


#define LIST_MAX_ITEMS 4
#define LIST_PARAM_SHOW_POS     80


OLED128X64::OLED128X64(sG2D* _screen,sLM::sLittleMenu* _menu){
    screen = _screen;
    menu = _menu;
}

OLED128X64::~OLED128X64(){
    // ...
}

// 实现渲染器接口
void OLED128X64::showMenuList(sLM::TreeNode* parent){
    if(parent == nullptr){
        LOG_ERR("错误,showList传入了空指针");
        return;
    }
    //获取当前父节点的数据,用来显示标题
    ItemData& parent_data = sLittleMenu::getNodeData(parent);

    

    //当前菜单项索引
    uint32_t curr_index = menu->getCurrMenuIndex();
    //y轴偏移
    uint32_t y_offset = 0;
    //行高
    static const uint16_t line_height = 12;
    //index偏移,用于滚动
    uint32_t scroll_index_offset = 0;

    //画一个装饰三角形在标题栏左边
    screen->drawTriangle(1,2,9,6,1,10,1);
    //标题字
    screen->write_string(15,3,parent_data.text);
    
    //右侧装饰线1
    uint16_t line1_x = 100;
    screen->drawLine(line1_x + 0,0,line1_x + 5 + 1,12,1);
    screen->drawLine(line1_x + 1,0,line1_x + 5 + 2,12,1);
    screen->drawLine(line1_x + 2,0,line1_x + 5 + 3,12,1);
    //右侧装饰线2
    uint16_t line2_x = 110;
    screen->drawLine(line2_x + 0,0,line2_x + 5 + 1,12,1);
    screen->drawLine(line2_x + 1,0,line2_x + 5 + 2,12,1);
    screen->drawLine(line2_x + 2,0,line2_x + 5 + 3,12,1);
    //反转标题栏
    screen->inv_area(0,0,128,12);

    //让下面的操作不影响标题栏
    y_offset += line_height + 5;
    //第一个item的上边框线
    screen->drawHLine(0,128,y_offset - 2,1);

    //第一个
    TreeNode* node = parent->child;
    //如果父节点没有子节点则不显示
    if(node == nullptr)return;

    //如果当前的菜单项大于能显示的菜单项了,就增加显示偏移
    //如果当前是最后一个项了,并且大于一个屏幕能显示的范围
    if(curr_index == menu->getCurrMenuNumber() && curr_index > (LIST_MAX_ITEMS - 1)){
        //让偏移量不减去-1,这样光标就能在最后一行
        scroll_index_offset = curr_index - LIST_MAX_ITEMS;
    }
    //如果当前索引值大于3了,就要往下滚动了
    else if(curr_index > (LIST_MAX_ITEMS - 1)){
        //这个-1就很微妙~~~ 可以实现一个神奇的功能!
        //如果curr_index=4,scroll_index_offset=1,从第一个索引开始显示,也就是"往下滚动了一页"
        scroll_index_offset = curr_index - (LIST_MAX_ITEMS - 1);
    }
    
    //只能显示四个
    for(int i = 1;i <= LIST_MAX_ITEMS;i++){
        //获取当前节点
        TreeNode* node = menu->getIndexMenu(scroll_index_offset + i);
        //获取当前节点的数据
        ItemData& data = sLittleMenu::getNodeData(node);

        if(data.param.access == ParamAccess::RO){
            if(data.param.update_cb)data.param.update_cb(&(data.param.val_i),data.param.param_tag);
        }

        //绘制行的标题
        screen->write_string(0, y_offset + 1, data.text);
        screen->drawHLine(0,128,y_offset + 10,1);
        char str[10];
        if(node->child){
            snprintf(str,10,">");
            screen->write_string(100,y_offset + 1,str);
            screen->write_string(101,y_offset + 1,str);
            screen->write_string(102,y_offset + 1,str);
        }else{
            if(data.type == ItemType::NORMAL && data.param.access != ParamAccess::NO){
                if(data.param.type == ParamType::FLOAT){
                    float& value  = *(float*)&(data.param.val_i);
                    char* unit    =  (char*)&(data.param.unit);
                    snprintf(str,10,data.param.show_fmt,value,unit);
                }
                else if(data.param.type == ParamType::INT){
                    int& value  = *(int*)&(data.param.val_i);
                    char* unit  =  (char*)&(data.param.unit);
                    snprintf(str,10,data.param.show_fmt,value,unit);
                }
                else if(data.param.type == ParamType::STRING){
                    char* value  =  (char*)&(data.param.val_i);
                    snprintf(str,10,"%s",value);
                }
                screen->write_string(LIST_PARAM_SHOW_POS,y_offset + 1,str);
            }
            else if(data.type == sLM::ItemType::SWITCH){

            }
        }
        
        //处理选中区域
        if(data.is_hover == true){
            screen->inv_area(0,y_offset - 1 ,128,y_offset + 9);
        }
        if(data.is_selected == true){
            screen->inv_area(LIST_PARAM_SHOW_POS - 5,y_offset - 1 ,125,y_offset + 9);
        }

        

        //如果下一个是空的了
        if(menu->getIndexMenu(scroll_index_offset + i + 1) == nullptr){
            return;
        }else{
            //下一行
            y_offset += line_height;
        }
        
    }
}


void OLED128X64::showWatingDialog(const char* _title, const char* _message){
    //画两个重叠矩形,让他看起来像个对话框
    screen->drawRectangle(5,5,122,65,1);
    screen->drawRectangle(1,1,118,61,0);

    uint16_t x_x = 107,x_y = 5;
    screen->drawLine(x_x + 0,x_y + 0,x_x + 5,x_y + 6,1);
    screen->drawLine(x_x + 1,x_y + 0,x_x + 6,x_y + 6,1);

    screen->drawLine(x_x + 5,x_y + 0,x_x + 0,x_y + 6,1);
    screen->drawLine(x_x + 6,x_y + 0,x_x + 1,x_y + 6,1);

    screen->printf(5,5,"%s",_title);
    screen->drawHLine(1,120,15,1);

    screen->printf(10,20,"\n%s",_message);
}


void OLED128X64::update(){
    if(menu->lock_info.status == true){
        showWatingDialog(menu->lock_info.tittle,menu->lock_info.message);
    }else{
        showMenuList(menu->getCurr()->parent);
    }
    
}


