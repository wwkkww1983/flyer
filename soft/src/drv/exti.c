/******************************************************************************
 *
 * 文件名  ： exti.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160412 
 * 版本号  ： 1.0
 * 文件描述： 外部中断模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"
#include "board.h"
#include "stm32f4xx_hal.h"
#include "exti.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* 中断回调 */
func_T s_callback = NULL;
void *s_callback_argv = NULL;

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
/* exti在Msp_Init过程配置管腿 */
void exti_init(void){;}

/* 设置回调 */
void exti_set_callback(func_T callback, void *argv)
{
    s_callback = callback;
    s_callback_argv = argv;
}

/* 覆盖stm32f4xx.s中弱符号 */
void SENSOR_INT_EXTI_IRQHandler(void)
{
    /* 判断中断 */
    if(__HAL_GPIO_EXTI_GET_IT(SENSOR_INT_PIN) != RESET)
    {
        /* 清中断 */
        __HAL_GPIO_EXTI_CLEAR_IT(SENSOR_INT_PIN); 
        
        /* 回调 */
        if(NULL != s_callback)
        {
            s_callback(s_callback_argv);
        }
    }
}

