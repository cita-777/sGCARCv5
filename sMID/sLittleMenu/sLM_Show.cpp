#include "sLM_Show.hpp"


sLM_OLEDWeights slm_weights;

sLM_OLEDWeights::sLM_OLEDWeights(){
    
}

sLM_OLEDWeights::~sLM_OLEDWeights(){

}

// extern sLittleMenu lm;

void sLM_OLEDWeights::init(sG2D* _screen,sLittleMenu* _menu){
    screen = _screen;
    menu = _menu;
}

void toUpper(char str[]) {
    for (int i = 0; str[i] != '\0'; ++i) {
        str[i] = toupper((unsigned char)str[i]); // 转换为大写
    }
}



void sLM_OLEDWeights::showList(sLM_TreeNode* parent){
    //如果当前节点的父节点是空的,说明目前在root节点
    if(parent == nullptr){
        parent = parent->child;
    }
    //获取当前父节点的数据,用来显示标题
    sLM::MenuItemData& parent_data = sLM::getNodeData(parent);

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
    sLM_TreeNode* node = parent->child;

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
        sLM_TreeNode* node = menu->getIndexMenu(scroll_index_offset + i);
        //获取当前节点的数据
        sLM::MenuItemData& data = sLM::getNodeData(node);

        if(data.param.access == sLM::Item_ParamAccess::RO){
            if(data.param.update_cb)data.param.update_cb(&(data.param.value),data.param.param_tag);
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
            if(data.param.type == sLM::Item_ParamType::FLOAT){
                float& value  = *(float*)&(data.param.value);
                char* unit    =  (char*)&(data.param.unit);
                snprintf(str,10,SLM_WEIGHTS_LIST_PARAM_SHOW_FLOAT,value,unit);
            }
            else if(data.param.type == sLM::Item_ParamType::INT){
                int& value  = *(int*)&(data.param.value);
                char* unit  =  (char*)&(data.param.unit);
                snprintf(str,10,SLM_WEIGHTS_LIST_PARAM_SHOW_INT,value,unit);
            }
            screen->write_string(SLM_WEIGHTS_LIST_PARAM_SHOW_POS,y_offset + 1,str);
            
        }
        
        //处理选中区域
        if(data.is_hover == true){
            screen->inv_area(0,y_offset - 1 ,128,y_offset + 9);
        }
        if(data.is_selected == true){
            screen->inv_area(SLM_WEIGHTS_LIST_PARAM_SHOW_POS - 5,y_offset - 1 ,125,y_offset + 9);
        }

        

        //如果下一个是空的了
        if(menu->getIndexMenu(scroll_index_offset + i + 1) == nullptr){
            // oled.drawHLine(0,128,y_offset + 1,1);
            // oled.write_string(10, y_offset + 3," ----NO MORE----");
            return;
        }else{
            //下一行
            y_offset += line_height;
        }
        
    }
}
