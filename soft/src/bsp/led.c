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
 *            2.  增加board.h中灯的定义
 *            3.  增加g_led_list中灯的定义
 *            4.  增加board.c中灯的GPIO的时钟初始化
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include "config.h"
#include "board.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "led.h"
#include "debug.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* led 表 */
LED_LIST_T g_led_list[] = {
    {MLED, MLED_GPIO_PIN}
};

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
void led_init(void)
{
    /* 基本初始化位于 board.c Msp_Init */
}

/* 低灭 高亮 */
/* 亮灯 */
void led_on(LED_NAME led)
{
    if(led > LED_MAX)
    {
        while(1);
    }

    HAL_GPIO_WritePin(g_led_list[led].port, g_led_list[led].pin, GPIO_PIN_RESET); 
}

/* 灭灯 */
void led_off(LED_NAME led)
{
    if(led > LED_MAX)
    {
        while(1);
    }

    HAL_GPIO_WritePin(g_led_list[led].port, g_led_list[led].pin, GPIO_PIN_SET);
}

/* 状态翻转 */
void led_toggle(LED_NAME led)
{
    if(led > LED_MAX)
    {
        while(1);
    }

    HAL_GPIO_TogglePin(g_led_list[led].port, g_led_list[led].pin);
}

void led_test(void)
{
    /* led 测试 */
    int i = 0;
    int j = 0;
    int delay = 200;     /* led测试延迟ms */
    int flash_times = 5; /* 闪烁次数 不包括off/on测试 */

    debug_log("观察led是否闪烁.\r\n"); 
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

