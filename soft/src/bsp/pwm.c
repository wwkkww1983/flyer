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

/************************************ 头文件 ***********************************/
#include "config.h"
#include "board.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "pwm.h"

#define  PERIOD_VALUE       (uint32_t)(666 - 1)  /* Period Value  */
#define  PULSE1_VALUE       (uint32_t)(PERIOD_VALUE*0.0/100)        /* Capture Compare 1 Value  */
#define  PULSE2_VALUE       (uint32_t)(PERIOD_VALUE*33.0/100) /* Capture Compare 2 Value  */
#define  PULSE3_VALUE       (uint32_t)(PERIOD_VALUE*77.0/100)        /* Capture Compare 3 Value  */
#define  PULSE4_VALUE       (uint32_t)(PERIOD_VALUE*100.0/100) /* Capture Compare 4 Value  */

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
uint32_t s_uhPrescalerValue = 0;

static TIM_HandleTypeDef s_tim_handle;
static TIM_OC_InitTypeDef s_sConfig;

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
void pwm_init(void)
{ 
    /* Compute the prescaler value to have TIM3 counter clock equal to 15000000 Hz */
    s_uhPrescalerValue = (uint32_t)((SystemCoreClock/2) / 15000000) - 1; 
    
    /*##-1- Configure the TIM peripheral #######################################*/
    /* -----------------------------------------------------------------------
    TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles.

      In this example TIM3 input clock (TIM3CLK) is set to APB1 clock x 2,
      since APB1 prescaler is equal to 2.
        TIM3CLK = APB1CLK*2
        APB1CLK = HCLK/2
        => TIM3CLK = HCLK = SystemCoreClock

      To get TIM3 counter clock at 15 MHz, the prescaler is computed as follows:
         Prescaler = (TIM3CLK / TIM3 counter clock) - 1
         Prescaler = ((SystemCoreClock) /15 MHz) - 1

      To get TIM3 output clock at 22,52 KHz, the period (ARR)) is computed as follows:
         ARR = (TIM3 counter clock / TIM3 output clock) - 1
             = 665

      TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR + 1)* 100 = 50%
      TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR + 1)* 100 = 37.5%
      TIM3 Channel3 duty cycle = (TIM3_CCR3/ TIM3_ARR + 1)* 100 = 25%
      TIM3 Channel4 duty cycle = (TIM3_CCR4/ TIM3_ARR + 1)* 100 = 12.5%

      Note:
       SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
       Each time the core clock (HCLK) changes, user had to update SystemCoreClock
       variable value. Otherwise, any configuration based on this variable will be incorrect.
       This variable is updated in three ways:
        1) by calling CMSIS function SystemCoreClockUpdate()
        2) by calling HAL API function HAL_RCC_GetSysClockFreq()
        3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
    ----------------------------------------------------------------------- */
    /* Initialize TIMx peripheral as follows:
         + Prescaler = (SystemCoreClock / 15000000) - 1
         + Period = (666 - 1)
         + ClockDivision = 0
         + Counter direction = Up */
    s_tim_handle.Instance = TIMx;
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
		
    //while (1);

    //return;
}

void pwm_test(void)
{
    ;
}

