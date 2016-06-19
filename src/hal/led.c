/******************************************************************************
 *
 * 文件名  ： led.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160412 
 * 版本号  ： 1.0
 * 文件描述： led模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "config.h"
#include "board.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "led.h"


/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
LED_LIST_T s_led_list[] = {
    {NULL, 0}, /* 不使用 */
    {LED1x, LED1x_GPIO_PIN},
    {LED2x, LED2x_GPIO_PIN},
    {LED3x, LED3x_GPIO_PIN},
    {LED4x, LED4x_GPIO_PIN}
};

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
/* led使用gpio所以无Msp_Init过程配置管腿 所有初始化在该函数 */
void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    LED1x_CLK_ENABLE(); /* 时钟使能是宏,所以无法使用循环 */
    GPIO_InitStruct.Pin = s_led_list[1].pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(s_led_list[1].port, &GPIO_InitStruct); 

    LED2x_CLK_ENABLE();
    GPIO_InitStruct.Pin = s_led_list[2].pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(s_led_list[2].port, &GPIO_InitStruct); 

    LED3x_CLK_ENABLE();
    GPIO_InitStruct.Pin = s_led_list[3].pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(s_led_list[3].port, &GPIO_InitStruct); 

    LED4x_CLK_ENABLE();
    GPIO_InitStruct.Pin = s_led_list[4].pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(s_led_list[4].port, &GPIO_InitStruct); 

    /* led 测试 */
#if 1
    int i = 0;
		int j = 0;
    int delay = 100;
    /* 上电默认点亮
     * GPIO低电平 */
    HAL_Delay(delay);
    /* 此处熄灭*/
    for(i = 1; i < 5; i++)
    {
        led_off(i); 
    } 
    HAL_Delay(delay);

    /* 恢复点亮 */
    for(i = 1; i < 5; i++)
    {
        led_on(i); 
    } 

    /* 闪5次 */
    while(j++<5)
    {
        for(i = 1; i < 5; i++)
        {
            led_toggle(i); 
        } 
        HAL_Delay(delay);
    }
#endif

}

/* 低灭 高亮 */
/* 
 * 点灯
 * 1 <= num <= 4 
 * */
void led_on(int32_T num)
{
    if((num < 1)
    || (num > 4))
    {
        while(1);
    }

    HAL_GPIO_WritePin(s_led_list[num].port, s_led_list[num].pin, GPIO_PIN_RESET); 
}

/* 
 * 灭灯
 * 1 <= num <= 4 
 * */
void led_off(int32_T num)
{
    if((num < 1)
    || (num > 4))
    {
        while(1);
    }

    HAL_GPIO_WritePin(s_led_list[num].port, s_led_list[num].pin, GPIO_PIN_SET);
}

/* 
 * 状态翻转
 * 1 <= num <= 4 
 * */
void led_toggle(int32_T num)
{
    if((num < 1)
    || (num > 4))
    {
        while(1);
    }

    HAL_GPIO_TogglePin(s_led_list[num].port, s_led_list[num].pin);
}

