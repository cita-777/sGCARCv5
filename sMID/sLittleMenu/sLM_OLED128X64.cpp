#include "sLM_OLED128X64.hpp"




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


void OLED128X64::drawListFrame(const char* tittle){
    /*绘制标题栏,这些是不随着滚动而滚动的*/
    //画一个装饰三角形在标题栏左边
    screen->drawTriangle(1,2,9,6,1,10,1);
    //标题字
    screen->printf(15,3,tittle);
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
    screen->revArea(0,0,128,12);
}

// 实现渲染器接口
void OLED128X64::showMenuList(sLM::ItemBase* parent){
    if(parent == nullptr){
        SLM_LOG_ERR("错误,showList传入了空指针");
        return;
    }

    /*显示标题字*/
    ItemType parent_type = parent->getItemType();
    const char* parent_tittle = nullptr;
    if(parent_type == ItemType::ENTERABLE){
        parent_tittle = static_cast<EnterableItem*>(parent)->getTittle();
    }
    drawListFrame(parent_tittle);

    //当前菜单项索引
    uint32_t curr_index = menu->getCurrMenuIndex();
    //y轴偏移
    uint32_t y_offset = 0;
    //行高
    static const uint16_t line_height = 12;
    //index偏移,用于滚动
    uint32_t scroll_index_offset = 0;

    //让下面的操作不影响标题栏
    y_offset += line_height + 5;
    //第一个item的上边框线
    screen->drawHLine(0,128,y_offset - 2,1);

    //第一个
    ItemBase* item = parent->child;
    //如果父节点没有子节点则不显示
    if(item == nullptr)return;

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
        ItemBase* curr_item = menu->getIndexMenu(scroll_index_offset + i);
        /*获取当前节点的类型*/
        ItemType curr_type = curr_item->getItemType();
        //获取当前的tittle 虚函数表,启动!
        const char* curr_tittle = curr_item->getTittle();

        /*绘制item行的标题*/
        screen->printf(0, y_offset + 1, curr_tittle);   screen->drawHLine(0,128,y_offset + 10,1);

        /*处理item不同的部分*/
        //对EnterableItem来说,右边显示一个箭头
        if(curr_type == ItemType::ENTERABLE){
            char arrow[2];
            snprintf(arrow,2,">");
            screen->printf(100,y_offset + 1,arrow);
            screen->printf(101,y_offset + 1,arrow);
            screen->printf(102,y_offset + 1,arrow);
        }
        //对Label,什么都不做
        else if(curr_type == ItemType::LABEL){

        }
        //对Button
        else if(curr_type == ItemType::BUTTON){
            ButtonItem* button = static_cast<ButtonItem*>(curr_item);
            //绘制cover_text
            screen->printf(LIST_PARAM_SHOW_POS,y_offset + 1,button->getCoverText());
            //绘制一个小方块表示button
            screen->revArea(LIST_PARAM_SHOW_POS - 5,y_offset + 0,120,y_offset + 8);
            
        }
        //对switch,画一个类似于GUI里的那种二值开关
        else if(curr_type == ItemType::SWITCH){
            SwitchItem* switch_item = static_cast<SwitchItem*>(curr_item);
            if(switch_item->getStatus()){
                //绘制text
                screen->printf(LIST_PARAM_SHOW_POS + 0,y_offset + 1,switch_item->getText());
                //绘制一个小方块表示button
                screen->revArea(LIST_PARAM_SHOW_POS - 5,y_offset + 0,120,y_offset + 8);
            }else{
                screen->drawRectangle(LIST_PARAM_SHOW_POS - 5,y_offset + 0,120,y_offset + 8,0);
                screen->printf(LIST_PARAM_SHOW_POS + 15,y_offset + 1,switch_item->getText());
            }
        }
        //对IntValAdj
        else if(curr_type == ItemType::INT_VAL_ADJ){
            IntValAdj* int_val_adj = static_cast<IntValAdj*>(curr_item);
            screen->printf(LIST_PARAM_SHOW_POS,y_offset + 1,int_val_adj->getValText());
        }
        else if(curr_type == ItemType::FLOAT_VAL_ADJ){
            FloatValAdj* float_val_adj = static_cast<FloatValAdj*>(curr_item);
            screen->printf(LIST_PARAM_SHOW_POS,y_offset + 1,float_val_adj->getValText());
        }


        /*处理选中区域(光标)*/
        if(curr_item->is_hover)screen->revArea(0,y_offset - 1 ,128,y_offset + 9);
        if(curr_item->is_selected)screen->revArea(LIST_PARAM_SHOW_POS - 5,y_offset - 1 ,125,y_offset + 9);

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
        // if(menu->getNodeData(menu->getCurr()->parent).child_show_mode == MenuChildShowMode::LIST){
        //     showMenuList(menu->getCurr()->parent);
        // }

        // if(menu->getCurr()->parent)
        if(menu->curr->getItemType() == ItemType::ENTERABLE){
            EnterableItem* item = static_cast<EnterableItem*>(menu->curr);
            if(item->getChildShowType() == ItemShowType::CANVAS && !item->is_hover){
                oled.clear();
                item->CallCanvasPeriodciallyCallback();
                return;
            }
        }
        
        showMenuList(menu->getCurr()->parent);
        
    }


    
}


