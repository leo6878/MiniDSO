/********************* (C) COPYRIGHT 2016 e-Design Co.,Ltd. ********************
File Name :      UI.c
Version :        1.7a
Description:
Author :         Ning
Data:            2017/11/22
History:   
2017/04/01       显示挡位在转动和待机时;
2017/08/01       显示转动时电流大小;
2017/09/10       显示充电时充电进度;
*******************************************************************************/
#include "Oled.h"
#include "Oledfont.h"
#include "UI.h"
#include "string.h"
#include "ctrl.h"
#include "own_sprintf.h"
#include "stdio.h"//2017.8.2显示电流测试
#include "adc.h"

u8 gScrew_position = 17;/*螺丝图像显示位置*/
extern u16 The_Current;//2017.8.1当前电流
extern vu16 AWD_entry;
/*******************************************************************************
函数名: Clear_Screen
函数作用:清屏
输入参数:NULL
返回参数:NULL
*******************************************************************************/
void Clear_Screen(void)
{
    u8 data[128];
    u8 i;
    
    memset(&data[0],0,128);
    for(i = 0; i < 2; i++)
        Oled_DrawArea(0, i * 8, 128, 8, data);
}
/*******************************************************************************
函数名: Display_Str
函数作用:显示16*16字符串
输入参数:x: 位置 str :显示字符串
返回参数:NULL
*******************************************************************************/
void Display_Str(u8 x,u8 y, char* str)
{
    u8* ptr;
    u8  temp;

    if((x < 1) || (x > 16))     x = 0;
    else                        x--;
    while(*str != 0) 
    {
        temp = *str++ - ' ';//得到偏移后的值
        ptr = (u8*)ASCII6x8;
        ptr += temp * 6;
        Oled_DrawArea(x * 8, y, 6, 8, (u8*)ptr);
        x++;
    }
}
/*******************************************************************************
函数名: Show_SetLv
函数作用:显示设置档位
输入参数:gear 档位
返回参数:NULL
*******************************************************************************/
void Show_SetLv(u8 gear)
{
    u8* ptr;
    ptr = (u8*)SET_CNT + 96 * 2 * gear;

    Oled_DrawArea(0, 0, 96, 16, ptr);//绘制整屏幕
}
/*******************************************************************************
函数名: Low_Power
函数作用:显示低电量提醒
输入参数:NULL
返回参数:NULL
*******************************************************************************/
void Low_Power(void)
{
    u8 *ptr;
    ptr = (u8*)LOW_POWER;

    Oled_DrawArea(0, 0, 96, 16, ptr);
}
/*******************************************************************************
函数名: Show_BatV
函数作用:显示电量|电池状态
输入参数:step: 电量 Charging:电池状态
返回参数:NULL
*******************************************************************************/
void Show_BatV(u8 step, u8 Charging)
{
    u8 *ptr;
    static u8 min_v = 10;

    if(step > 10 && Charging == 0)      step = 10;
    if(Charging > 0)
    { //充电时，电量可以增加 即可以抖动
        if(step > min_v)
            min_v = step;
    }
    else //不在充电时
    {
        if(min_v >= step)       min_v = step;
        else                    step = min_v; //防止电量显示抖动
    }

    if(Charging == 0)
    { //不是充电
        ptr = (u8*)BAT_STAT + 20 * step;
        Oled_DrawArea(0, 0, 10, 16, ptr);
    }
    else if(Charging == 1)
    { //充电
        ptr = (u8*)BAT_STAT + 20 * (10 + step);
        Oled_DrawArea(0, 0, 10, 16, ptr);
    }
    else if(Charging == 2)
    {
        ptr = (u8*)BAT_STAT + 20 * 21;
        Oled_DrawArea(0, 0, 10, 16, ptr);
    }
}
/*******************************************************************************
函数名: Clear_Screw
函数作用:清屏
输入参数:NULL
返回参数:NULL
*******************************************************************************/
void Clear_Screw(void)
{
    u8 data[128], i;

    memset(&data[0], 0, 128);
    for(i = 0; i < 2; i++)
    {
        Oled_DrawArea(11, i * 8, 81, 8, data);//92 -16
    }
}
/*******************************************************************************
函数名: Show_ScrewMove
函数作用:动态显示螺丝图片
输入参数:status : Screw down status = 0 down，1up
返回参数:NULL
*******************************************************************************/
void Show_ScrewMove(u8 status)//Screw down status = 0 down，1up
{
    u8 *ptr;
    static u8 i = 1, step = 0;

    if(gScrew_position != i)    i = gScrew_position;
    if(status == 0)
    { //down
        ptr = (u8*)SD_IDF + 64 * step;
        Oled_DrawArea(11 + i, 0, 32, 16, ptr);
        i++;
        if(i >= 50 || i <= 0)   i = 0;//50-16
    }
    if(status == 1) 
    { //up
        ptr = (u8*)SD_IDF + 64 * step;
        Oled_DrawArea(11 + i, 0, 32, 16, ptr);
        i--;
        if(i <= 0 || i >= 50)   i = 49;//50-16
    }
    ptr = (u8*)LEVEL_IDF + 11 * 2 * info_def.torque_level;
    Oled_DrawArea(0, 0, 11, 16, ptr);//挡位
                                               
    gScrew_position = i;
    step++;
    if(step == 4)       step = 0;
    
    /*2017.8.1当前电流进度条*/
    if(step % 2)
    {
        if(The_Current > MAX_ROTATE_I)//超过堵转电流1200
        {
            ptr = (u8*)I_Size + 56;//电流大小显示条
            Oled_DrawArea(93, 0, 2, 16, ptr);
        }
        else if(The_Current == 0)//0
        {
            ptr = (u8*)I_Size;
            Oled_DrawArea(93, 0, 2, 16, ptr);
        }
        else
        {
            ptr = (u8*)I_Size + (The_Current / 85 * 4);   
            Oled_DrawArea(93, 0, 2, 16, ptr);
        }
    }
}
/*******************************************************************************
函数名: Show_Screw
函数作用:固定位置显示螺丝
输入参数:u8 mode  0:空闲模式   1:工作模式下的固定显示
返回参数:NULL
*******************************************************************************/
void Show_Screw(u8 mode)
{
    u8 *ptr;
    if(mode)
    {
        ptr = (u8*)LEVEL_IDF + 11 * 2 * info_def.torque_level;
        Oled_DrawArea(0, 0, 11, 16, ptr);//挡位
        ptr = (u8*)CONSULT + 8;
        Oled_DrawArea(92, 0, 4, 16, ptr);//底座
    }
    else
    {
        ptr = (u8*)LEVEL_IDF + 11 * 2 * info_def.torque_level;
        Oled_DrawArea(75, 0, 11, 16, ptr);//挡位
        ptr = (u8*)CONSULT;
        Oled_DrawArea(92, 0, 4, 16, ptr);//底座
    }
    if(mode != 2)
    {
        ptr = (u8*)SD_IDF;
        Oled_DrawArea(27, 0, 32, 16, ptr);//螺丝
        gScrew_position = 17;  
    }
}
/******************************** END OF FILE *********************************/
