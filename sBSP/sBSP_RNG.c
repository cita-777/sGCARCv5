#include "sBSP_RNG.h"

/**
 * sBSP_RNG.c
 * 
 * 241206 v1.0 bySightseer.
 * 
 */

RNG_HandleTypeDef hrng;


void sBSP_RNG_Init(){
    __HAL_RCC_RNG_CLK_ENABLE();

    hrng.Instance = RNG;
    HAL_RNG_Init(&hrng);
    
}


uint32_t sBSP_RNG_GetU32(){
    uint32_t num = 0;
    HAL_RNG_GenerateRandomNumber(&hrng,&num);
    return num;
}

int32_t sBSP_RNG_Get32(){

}

uint16_t sBSP_RNG_GetU16(){
    uint32_t num = 0;
    HAL_RNG_GenerateRandomNumber(&hrng,&num);
    return (uint16_t)(num & 0x0000FFFF);
}

int16_t sBSP_RNG_Get16(){

}

uint8_t sBSP_RNG_GetU8(){
    uint32_t num = 0;
    HAL_RNG_GenerateRandomNumber(&hrng,&num);
    return (uint8_t)(num & 0x000000FF);
}

int8_t sBSP_RNG_Get8(){

}

float sBSP_RNG_GetRangeFloat(float min,float max){
    uint32_t num = RNG->DR;
    HAL_RNG_GenerateRandomNumber(&hrng,&num);

    //将随机数缩放到[0,1]范围
    float nor = (float)num / (float)0xFFFFFFFF;

    //缩放到[min, max]范围
    return min + nor * (max - min);
}

