#include "sG2D.hpp"


#include "sDRV_GenOLED.h"
#include "sBSP_DMA.h"

#include <stdarg.h>


/**
 * sG2D.cpp
 * Sightseer's 2D Graphic Library
 * Sightseer用于OLED的2D图形库
 * 
 * 
 * 更新记录
 * v1.0 TIME 忘了
 * 初版
 * 
 * v1.1 TIME 2024.06.17inHNIP9607Lab
 * 优化性能,使用DMA
 * 
 * v1.2 TIME 2024.10.31inHNIP9607Lab
 * 1.进一步优化性能,修改了GRAM排列方式,以便DMA发送,优化架构,DMA可选
 * 2.使用双1KB全屏缓冲区,在刷屏时同时进行绘制
 * 
 * v1.3 250123
 * 优化代码结构
 * 
 */




sG2D oled;

/*********************接口*********************/

static inline uint32_t portGetTick(){
    return HAL_GetTick();
}

static inline void* portMalloc(size_t size){
    return pvPortMalloc(size);
}

static inline void portFree(void* pv){
    vPortFree(pv);
}

#ifdef SG2D_USE_DMA_MEMSET
    static inline void portM2M_DMA_Init(){
        sBSP_DMA_MemSetByte_Init();
    }

    static inline void portM2M_DMA_MemSetByte(uint8_t val,uint8_t* pDst,uint16_t len){
        sBSP_DMA_MemSetByte(val,pDst,len);
    }
#endif

static inline void portUpdateScreen(uint8_t* buf){
    sDRV_GenOLED_UpdateScreen(buf);
}



sG2D::sG2D(){

}

int sG2D::init(uint16_t _scr_w,uint16_t _scr_h){
    //注册屏幕信息
    scr_w = _scr_w; scr_h = _scr_h;
    buf_size = (scr_h * scr_w) / 8;
    //创建缓冲区
    draw_buf = (uint8_t*)portMalloc(buf_size);
    disp_buf = (uint8_t*)portMalloc(buf_size);

    if(!draw_buf || !disp_buf){
        return -1;
    }

    #ifdef SG2D_USE_DMA_MEMSET
        portM2M_DMA_Init();
    #endif

    return 0;
}


sG2D::~sG2D(){
    portFree(draw_buf);
    portFree(disp_buf);
}


void sG2D::setDot(uint16_t x,uint16_t y,bool dot_en){
    if((x >= scr_w) || (y >= scr_h)) return;
    if(dot_en){
        //和排列方式有关,这里是从左到右,一行行连续排列的
        draw_buf[x + scr_w * (y / 8)] |=   1 << (y % 8);
    }else{
        draw_buf[x + scr_w * (y / 8)] &= ~(1 << (y % 8));
    }
}

bool sG2D::getDot(uint16_t x, uint16_t y){
    if((x >= scr_w) || (y >= scr_h)) return 0;
    return (draw_buf[x + scr_w * (y / 8)]& (1 << (y % 8))) != 0;
}

void sG2D::revArea(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
    uint8_t x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            uint8_t currentDot = getDot(x, y);
            setDot(x, y, !currentDot);  //翻转像素状态
        }
    }
}

//写入一竖列(8个点)
void sG2D::set_byte(uint16_t x, uint16_t y, uint8_t data){
    for(uint8_t i = 0;i < 8;i++){
        if(data & (1 << i)){
            setDot(x,y + i,1);
        }
    }
}

void sG2D::write_char(uint16_t x,uint16_t y,sG2D_Font_D5x7_t user_char){
    set_byte(x + 1,y,user_char.CharByte0);
    set_byte(x + 2,y,user_char.CharByte1);
    set_byte(x + 3,y,user_char.CharByte2);
    set_byte(x + 4,y,user_char.CharByte3);
    set_byte(x + 5,y,user_char.CharByte4);
}

void sG2D::writeNumber(uint8_t x,uint8_t y,uint32_t num){
    uint8_t length = (num == 0)? 1 : (uint8_t)(log10(num) + 1);
    for(uint8_t i = length;i > 0;i--){
        write_char(x + (i * 6) - 6,y,SymbolFont[num / (uint16_t)powf(10 , length - i) % 10]);
    }
}

void sG2D::write_string(uint16_t x,uint16_t y,const char* str){
    int16_t x_offset = 0;
    //遍历字符串
    while(*str != '\0'){
        if(x != 128){
            if(*str == '\n'){
                x_offset = -6;
                y += 9; //换行
            }
            set_byte(x + x_offset + 1,y,CharFont[*str].CharByte0);
            set_byte(x + x_offset + 2,y,CharFont[*str].CharByte1);
            set_byte(x + x_offset + 3,y,CharFont[*str].CharByte2);
            set_byte(x + x_offset + 4,y,CharFont[*str].CharByte3);
            set_byte(x + x_offset + 5,y,CharFont[*str].CharByte4);
            x_offset += 6;
            str++; 
        }else{
            //已经达到所能显示的最大范围了
            return;
        }
    }
}

void sG2D::printf(uint16_t x,uint16_t y,const char* fmt,...){
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf,fmt,args);
    va_end(args);
    
    write_string(x,y,buf);
}

//绘制任意线段
void sG2D::drawLine(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,bool dot_en){
    //Bresenham直线绘制算法
    int16_t dx = abs(x1 - x0);
    int16_t dy = -abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;

    while (1) {
        setDot(x0, y0, dot_en); //设置像素点
        if(x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if(e2 >= dy){
            err += dy;
            x0 += sx;
        }
        if(e2 <= dx){
            err += dx;
            y0 += sy;
        }
    }
}


void sG2D::drawVLine(uint16_t x, uint16_t y0, uint16_t y1, bool dot_en){
    for(uint16_t i = y0; i < y1; i++){
        setDot(x,i,dot_en);
    }
}

void sG2D::drawHLine(uint16_t x0, uint16_t x1,uint16_t y, bool dot_en){
    for(uint16_t i = x0; i < x1; i++){
        setDot(i,y,dot_en);
    }
}

//绘制三角形
void sG2D::drawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool dot_en){
    drawLine(x0, y0, x1, y1, dot_en);
    drawLine(x1, y1, x2, y2, dot_en);
    drawLine(x2, y2, x0, y0, dot_en);
}

//绘制矩形
void sG2D::drawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, bool is_fill){
    if (is_fill) {
        for (uint16_t y = y0; y <= y1; y++) {
            drawHLine(x0, x1, y, 1);
        }
    } else {
        for (uint16_t y = y0 - 1; y < y1; y++) {
            drawHLine(x0 - 0, x1 - 0, y, 0);
        }
        drawHLine(x0, x1, y0, 1);
        drawHLine(x0, x1, y1, 1);
        drawVLine(x0, y0, y1, 1);
        drawVLine(x1, y0, y1 + 1, 1);
    }
}

void sG2D::clear(){
    setAll(0);
}


void sG2D::setFPSMode(FPS_ShowMode mode){
    this->fps_mode = mode;
}


void sG2D::attachFPSInfo(){
    static uint32_t fps_ts_last;
    static uint32_t fps_ts_curr;
    static uint32_t fps_prid;
    static uint32_t fps_val;
    static uint8_t fps_x,fps_y;

    if(this->fps_mode == DIGITS2){
        fps_x = this->scr_w - 36u;
        fps_y = this->scr_h - 8u;
    }
    else if(this->fps_mode == DIGITS3){
        fps_x = this->scr_w - 42u;
        fps_y = this->scr_h - 8u;
    }
    
    fps_ts_last = fps_ts_curr;
    fps_ts_curr = portGetTick();
    fps_val = 1000 / (fps_ts_curr - fps_ts_last);

    drawRectangle(fps_x - 1,fps_y - 1 ,128,64,0);
    write_string(fps_x,fps_y,"FPS:");
    writeNumber(fps_x + 23,fps_y,fps_val);
    revArea(fps_x - 0,fps_y - 0 ,128,64);
}

void sG2D::setAll(bool px_en){
    if(px_en){
        #ifdef SG2D_USE_DMA_MEMSET
            portM2M_DMA_MemSetByte(0xFF,draw_buf,buf_size);
        #else
            memset(draw_buf,0xFF,buf_size);
        #endif
    }else{
        #ifdef SG2D_USE_DMA_MEMSET
            portM2M_DMA_MemSetByte(0x00,draw_buf,buf_size);
        #else
            memset(draw_buf,0x00,buf_size);
        #endif
    }
}

void sG2D::swap_buf(){
    //交换缓冲区
    uint8_t* tmp = disp_buf;
    disp_buf = draw_buf;
    draw_buf = tmp;
}


void sG2D::handler(){
    //检查上一帧是否刷完
    if(sDRV_GenOLED_IsIdle() == false) return;

    sDRV_GenOLED_SetShowEN(1);
    
    //附上FPS信息
    #ifdef SG2D_SHOW_FPS_INFO
        attachFPSInfo();
    #endif

    portUpdateScreen(draw_buf);
    //交换缓冲区,让DMA读取刷屏缓冲区,我写绘制缓冲区
    swap_buf();
}


