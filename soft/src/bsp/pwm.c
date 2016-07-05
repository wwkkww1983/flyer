/******************************************************************************
 *
 * 文件名  ： pwm.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160126 
 * 版本号  ： 1.0
 * 文件描述： pwm模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include "config.h"
#include "board.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "pwm.h"
#include "console.h"

#define  PERIOD_VALUE       (uint32_t)(666 - 1)  /* Period Value  */
#define  PULSE1_VALUE       (uint32_t)(PERIOD_VALUE*0.0/100)        /* Capture Compare 1 Value  */
#define  PULSE2_VALUE       (uint32_t)(PERIOD_VALUE*33.0/100) /* Capture Compare 2 Value  */
#define  PULSE3_VALUE       (uint32_t)(PERIOD_VALUE*77.0/100)        /* Capture Compare 3 Value  */
#define  PULSE4_VALUE       (uint32_t)(PERIOD_VALUE*100.0/100) /* Capture Compare 4 Value  */

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
PWM_LIST_T g_pwm_list[] = {
    {
        .name = PWM_FRONT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_1
    },
    {
        .name = PWM_RIGHT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_2
    },
    {
        .name = PWM_BACK,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_3
    },
    {
        .name = PWM_LEFT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_4
    }
};

/* tim句柄 */
static TIM_HandleTypeDef s_tim_handle;
/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
void pwm_init(void)
{ 
    static TIM_OC_InitTypeDef s_sConfig;
    uint32_t s_uhPrescalerValue = 0;

    /* Compute the prescaler value to have PWM_TIM counter clock equal to 15000000 Hz */
    s_uhPrescalerValue = (uint32_t)((SystemCoreClock/2) / 15000000) - 1; 
    
    /*##-1- Configure the TIM peripheral #######################################*/
    /* -----------------------------------------------------------------------
    PWM_TIM Configuration: generate 4 PWM signals with 4 different duty cycles.

      In this example PWM_TIM input clock (PWM_TIMCLK) is set to APB1 clock x 2,
      since APB1 prescaler is equal to 2.
        PWM_TIMCLK = APB1CLK*2
        APB1CLK = HCLK/2
        => PWM_TIMCLK = HCLK = SystemCoreClock

      To get PWM_TIM counter clock at 15 MHz, the prescaler is computed as follows:
         Prescaler = (PWM_TIMCLK / PWM_TIM counter clock) - 1
         Prescaler = ((SystemCoreClock) /15 MHz) - 1

      To get PWM_TIM output clock at 22,52 KHz, the period (ARR)) is computed as follows:
         ARR = (PWM_TIM counter clock / PWM_TIM output clock) - 1
             = 665

      PWM_TIM Channel1 duty cycle = (PWM_TIM_CCR1/ PWM_TIM_ARR + 1)* 100 = 50%
      PWM_TIM Channel2 duty cycle = (PWM_TIM_CCR2/ PWM_TIM_ARR + 1)* 100 = 37.5%
      PWM_TIM Channel3 duty cycle = (PWM_TIM_CCR3/ PWM_TIM_ARR + 1)* 100 = 25%
      PWM_TIM Channel4 duty cycle = (PWM_TIM_CCR4/ PWM_TIM_ARR + 1)* 100 = 12.5%

      Note:
       SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
       Each time the core clock (HCLK) changes, user had to update SystemCoreClock
       variable value. Otherwise, any configuration based on this variable will be incorrect.
       This variable is updated in three ways:
        1) by calling CMSIS function SystemCoreClockUpdate()
        2) by calling HAL API function HAL_RCC_GetSysClockFreq()
        3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
    ----------------------------------------------------------------------- */
    /* Initialize PWM_TIM peripheral as follows:
         + Prescaler = (SystemCoreClock / 15000000) - 1
         + Period = (666 - 1)
         + ClockDivision = 0
         + Counter direction = Up */
    s_tim_handle.Instance               = PWM_TIM;
    s_tim_handle.Init.Prescaler         = s_uhPrescalerValue;
    s_tim_handle.Init.Period            = PERIOD_VALUE;
    s_tim_handle.Init.ClockDivision     = 0;
    s_tim_handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    s_tim_handle.Init.RepetitionCounter = 0;
    if (HAL_TIM_PWM_Init(&s_tim_handle) != HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }

    /*##-2- Configure the PWM channels #########################################*/
    /* Common configuration for all channels */
    s_sConfig.OCMode       = TIM_OCMODE_PWM1;
    s_sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    s_sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    s_sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    s_sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    s_sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

    /* Set the pulse value for channel 1 */
    s_sConfig.Pulse = PULSE1_VALUE;
    if (HAL_TIM_PWM_ConfigChannel(&s_tim_handle, &s_sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
        /* Configuration Error */
        while(1);
    }

    /* Set the pulse value for channel 2 */
    s_sConfig.Pulse = PULSE2_VALUE;
    if (HAL_TIM_PWM_ConfigChannel(&s_tim_handle, &s_sConfig, TIM_CHANNEL_2) != HAL_OK)
    {
        /* Configuration Error */
        while(1);
    }

    /* Set the pulse value for channel 3 */
    s_sConfig.Pulse = PULSE3_VALUE;
    if (HAL_TIM_PWM_ConfigChannel(&s_tim_handle, &s_sConfig, TIM_CHANNEL_3) != HAL_OK)
    {
        /* Configuration Error */
        while(1);
    }

    /* Set the pulse value for channel 4 */
    s_sConfig.Pulse = PULSE4_VALUE;
    if (HAL_TIM_PWM_ConfigChannel(&s_tim_handle, &s_sConfig, TIM_CHANNEL_4) != HAL_OK)
    {
        /* Configuration Error */
        while(1);
    }

    /*##-3- Start PWM signals generation #######################################*/
    /* Start channel 1 */
    if (HAL_TIM_PWM_Start(&s_tim_handle, TIM_CHANNEL_1) != HAL_OK)
    {
        /* PWM Generation Error */
        while(1);
    }
    /* Start channel 2 */
    if (HAL_TIM_PWM_Start(&s_tim_handle, TIM_CHANNEL_2) != HAL_OK)
    {
        /* PWM Generation Error */
        while(1);
    }
    /* Start channel 3 */
    if (HAL_TIM_PWM_Start(&s_tim_handle, TIM_CHANNEL_3) != HAL_OK)
    {
        /* PWM generation Error */
        while(1);
    }
    /* Start channel 4 */
    if (HAL_TIM_PWM_Start(&s_tim_handle, TIM_CHANNEL_4) != HAL_OK)
    {
        /* PWM generation Error */
        while(1);
    }
		
    return;
}

void pwm_set(PWM_NAME pwm, int32_T val)
{
    console_printf("前右后左四个pwm分别为 0% 33% 77% 100%.\r\n"); 
}

void pwm_test(void)
{
    ;
}

