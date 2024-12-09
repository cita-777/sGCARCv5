#include "sLM_Show.hpp"




sLM_Show::sLM_Show(){

}

sLM_Show::~sLM_Show(){

}

extern sLittleMenu lm;

void sLM_Show::init(){

}

void toUpper(char str[]) {
    for (int i = 0; str[i] != '\0'; ++i) {
        str[i] = toupper((unsigned char)str[i]); // 转换为大写
    }
}

void sLM_Show::showList(sLM_TreeNode* parent){
    sLM::MenuItemData& parent_data = sLM::getNodeData(parent);
    //y轴偏移
    uint16_t y_offset = 0;
    //行高
    uint16_t line_height = 12;

    //画一个装饰三角形在标题栏左边
    oled.drawTriangle(1,2,9,6,1,10,1);
    //标题字
    //toUpper(parent_data.text);
    oled.write_string(15,3,parent_data.text);
    

    //右侧装饰线1
    uint8_t line1_x = 100;
    oled.drawLine(line1_x + 0,0,line1_x + 5 + 1,12,1);
    oled.drawLine(line1_x + 1,0,line1_x + 5 + 2,12,1);
    oled.drawLine(line1_x + 2,0,line1_x + 5 + 3,12,1);
    //右侧装饰线2
    uint8_t line2_x = 110;
    oled.drawLine(line2_x + 0,0,line2_x + 5 + 1,12,1);
    oled.drawLine(line2_x + 1,0,line2_x + 5 + 2,12,1);
    oled.drawLine(line2_x + 2,0,line2_x + 5 + 3,12,1);

    //反转标题栏
    oled.inv_area(0,0,128,12);

    //让下面的操作不影响标题栏
    y_offset += line_height + 5;
    //第一个item的上边框线
    oled.drawHLine(0,128,y_offset - 2,1);

    //第一个
    sLM_TreeNode* node = (parent->child);
    for(int i = 0;i < 4;i++){
        
        sLM::MenuItemData& data = sLM::getNodeData(node);
        //绘制行的标题
        oled.write_string(0, y_offset + 1, data.text);
        oled.drawHLine(0,128,y_offset + 10,1);
        char str[10];
        snprintf(str,10,"%d",data.param.i_val);
        oled.write_string(100,y_offset + 1,str);
        //处理选中区域
        if(data.is_hover == true){
            oled.inv_area(0,y_offset - 1 ,128,y_offset + 9);
        }
        if(data.is_selected == true){
            oled.inv_area(95,y_offset - 1 ,125,y_offset + 9);
        }

        

        //下一个
        node = node->next_sibling;
        //下一行
        y_offset += line_height;
        

    }


}

