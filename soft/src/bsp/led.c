/******************************************************************************
 *
 * 文件名  ： led.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160412 
 * 版本号  ： 1.0
 * 文件描述： led模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 加入led的方法:
 *            1.  增加LED_NAME中的灯编号
 *            2.  增加s_led_list中灯的定义
 *            3.  增加board.h中灯的定义
 *            4.  增加led_init中灯的GPIO的时钟初始化
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
    {MLED, MLED_GPIO_PIN}
};

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
/* led使用gpio所以无Msp_Init过程配置管腿 所有初始化在该函数 */
void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    int32_T i = 0;
    
    /* MLED 时钟使能 */
    MLED_CLK_ENABLE();
    /* 此处加入新增LED时钟使能 */

    for(i = 0; i < LED_MAX; i++)
    { 
        GPIO_InitStruct.Pin = s_led_list[i].pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
        HAL_GPIO_Init(s_led_list[i].port, &GPIO_InitStruct); 
    }
}

/* 低灭 高亮 */
/* 亮灯 */
void led_on(LED_NAME led)
{
    if(led > LED_MAX)
    {
        while(1);
    }

    HAL_GPIO_WritePin(s_led_list[led].port, s_led_list[led].pin, GPIO_PIN_RESET); 
}

/* 灭灯 */
void led_off(LED_NAME led)
{
    if(led > LED_MAX)
    {
        while(1);
    }

    HAL_GPIO_WritePin(s_led_list[led].port, s_led_list[led].pin, GPIO_PIN_SET);
}

/* 状态翻转 */
void led_toggle(LED_NAME led)
{
    if(led > LED_MAX)
    {
        while(1);
    }

    HAL_GPIO_TogglePin(s_led_list[led].port, s_led_list[led].pin);
}

void led_test(void)
{
    /* led 测试 */
    int i = 0;
    int j = 0;
    int delay = 100;     /* led测试延迟ms */
    int flash_times = 5; /* 闪烁次数 不包括off/on测试 */
    /* 上电默认点亮
     * GPIO低电平 */
    HAL_Delay(delay);
    /* 此处熄灭*/
    for(i = 0; i < LED_MAX; i++)
    {
        led_off((LED_NAME)i); 
    } 
    HAL_Delay(delay);

    /* 恢复点亮 */
    for(i = 0; i < LED_MAX; i++)
    {
        led_on((LED_NAME)i); 
    } 

    /* 闪5次 */
    while( j++ < flash_times )
    {
        for(i = 0; i < LED_MAX; i++)
        {
            led_toggle((LED_NAME)i); 
        } 
        HAL_Delay(delay);
    }
}

