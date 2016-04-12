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
#include "config.h"
#include "board.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "exti.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* 中断回调 */
func_T s_callback = NULL;
void *s_callback_argv = NULL;

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
/* exti模块使用gpio所以无Msp_Init过程配置管腿 所有初始化在该函数 */
void exti_init(void)
{ 
    GPIO_InitTypeDef GPIO_InitStruct;

    /* 设置中断 */
    IMU_INT_CLK_ENABLE();
    GPIO_InitStruct.Pin   = IMU_INT_PIN;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;

    HAL_GPIO_Init(IMU_INT_GPIO_PORT, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(IMU_INT_EXTI, PER_INT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(IMU_INT_EXTI);
}

/* 设置回调 */
void exti_set_callback(func_T callback, void *argv)
{
    s_callback = callback;
    s_callback_argv = argv;
}

/* 
 *
 * 覆盖STM32F4中弱符号
 * 硬件中断调用
 * 用户不调用 exti.h中不加
 *
 * */
void HAL_GPIO_EXTI_Callback(uint16_T pin)
{
    if((GPIO_PIN_13 == pin)
    && (NULL != s_callback))
    {
        s_callback(s_callback_argv);
    }
}

