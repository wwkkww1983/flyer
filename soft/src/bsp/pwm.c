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
#include "pwm.h"
#include "mpu9250.h"
#include "debug.h"
#include "lib_math.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
PWM_CH_T g_pwm_ch_list[] = {
    {
        /* 晶体顺时针方向第1个 */
        .name = PWM_FRONT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_1,
    },
    {
        /* 晶体顺时针方向第2个 */
        .name = PWM_RIGHT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_2,
    },
    {
        /* 晶体顺时针方向第3个 */
        .name = PWM_BACK,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_3,
    },
    {
        /* 晶体顺时针方向第4个 */
        .name = PWM_LEFT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_4,
    }
};

/* tim句柄 */
static TIM_HandleTypeDef s_tim_handle;
/* pwm通道配置 */
static TIM_OC_InitTypeDef s_sConfig;
/* PWM一次脉冲的周期 */
static int32_T s_period = 0; 

/********************************** 函数声明区 *********************************/
static void pwm_set(PWM_NAME pwm, int32_T val);
static void pwm_test(void);

/********************************** 函数实现区 *********************************/
void pwm_init(void)
{
    uint32_T pre_scale_val = 0;

    /* 
     *
     * 将计数器时钟设置为:15MHz, 计算预分频值
     * PWM_TIMCLK使用 APB1 的时钟 即 HCLK/2
     *
     * 预分频值:
     *    pre_scale_val = (APB1_CLK / PWM_TIM_CLK ) - 1
     * => pre_scale_val = ((SystemCoreClock / 2 ) /15 MHz) - 1
     *
     */
    pre_scale_val = (uint32_t)((SystemCoreClock/2) / 15000000) - 1; 

    /*
     * 设置周期为:1000 便于编程
     * ARR = (PWM_TIM源时钟/PWM_TIM输出频率) - 1
     * => PWM_TIM输出频率 = PWM_TIM源时钟 / (ARR + 1)
     * ARR = 1000
     * PWM_TIM源时钟:15MHz
     * => PWM_TIM输出频率 = 15000000 / (1000 + 1)
     *   = 14.985 kHz
     *
     * 设置PWM_TIM输出频率为20 KHz, 周期(ARR)为:
     * ARR = (PWM_TIM源时钟/PWM_TIM输出频率) - 1
     * = 749
     * */
     s_period = 1000;

    /* 配置PWM_TIM计数器:
     * Initialize PWM_TIM peripheral as follows:
     * Prescaler = (SystemCoreClock / 2 / 15000000) - 1
     * Period = (750 - 1)
     * ClockDivision = 0
     * Counter direction = Up
     * */
    s_tim_handle.Instance               = PWM_TIM;
    s_tim_handle.Init.Prescaler         = pre_scale_val;
    s_tim_handle.Init.Period            = s_period;
    s_tim_handle.Init.ClockDivision     = 0;
    s_tim_handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    s_tim_handle.Init.RepetitionCounter = 0;
    if (HAL_TIM_PWM_Init(&s_tim_handle) != HAL_OK)
    {
        while(1);
    }

    /* 各通道的占空比计算如下:
     * PWM_TIM_Channel1 占空比 = (PWM_TIM_CCR1/ PWM_TIM_ARR + 1)* 100
     * PWM_TIM Channel2 占空比 = (PWM_TIM_CCR2/ PWM_TIM_ARR + 1)* 100
     * PWM_TIM Channel3 占空比 = (PWM_TIM_CCR3/ PWM_TIM_ARR + 1)* 100
     * PWM_TIM Channel4 占空比 = (PWM_TIM_CCR4/ PWM_TIM_ARR + 1)* 100 
     *
     * 初始化的时候占空比全0
     */
    /* 所有通道都需要 */
    s_sConfig.OCMode       = TIM_OCMODE_PWM1;
    s_sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    s_sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    s_sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    s_sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    s_sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET; 

    /* 测试电机 */
    pwm_test();
    /* 关闭电机 */
    pwm_motor_off();

    return;
}

static void pwm_set(PWM_NAME pwm, int32_T val)
{
    int32_T period = 0;

    /* 参数检查 */
    if(pwm > PWM_MAX)
    {
        while(1);
    }

    /* 限制val在有效值范围内 [0,period] */
    period = pwm_get_period();
    if(val < 0)
    {
        val = 0;
    }
    if(val > period)
    {
        val = period;
    }

    /* 修改占空比 */
    s_sConfig.Pulse = val;
    if (HAL_TIM_PWM_ConfigChannel(&s_tim_handle, &s_sConfig, g_pwm_ch_list[pwm].ch) != HAL_OK)
    {
        while(1);
    } 

    /* 启动PWM */
    if (HAL_TIM_PWM_Start(&s_tim_handle, g_pwm_ch_list[pwm].ch) != HAL_OK)
    {
        while(1);
    }
}

static void pwm_test(void)
{
    int32_T period = 0;
    int32_T accelerator_rate = 5;
    int32_T delay = 1000;

    debug_log("前右后左分别%d%%油门运动%dms.\r\n", accelerator_rate, delay);

    period = pwm_get_period();

    pwm_set(PWM_FRONT, (int32_T)(period * accelerator_rate / 100.0));
    pwm_set(PWM_RIGHT, (int32_T)(period * 0.0));
    pwm_set(PWM_BACK,  (int32_T)(period * 0.0));
    pwm_set(PWM_LEFT,  (int32_T)(period * 0.0));
    HAL_Delay(delay);

    pwm_set(PWM_FRONT, (int32_T)(period * 0.0));
    pwm_set(PWM_RIGHT, (int32_T)(period * accelerator_rate / 100.0));
    pwm_set(PWM_BACK,  (int32_T)(period * 0.0));
    pwm_set(PWM_LEFT,  (int32_T)(period * 0.0));
    HAL_Delay(delay);

    pwm_set(PWM_FRONT, (int32_T)(period * 0.0));
    pwm_set(PWM_RIGHT, (int32_T)(period * 0.0));
    pwm_set(PWM_BACK,  (int32_T)(period * accelerator_rate / 100.0));
    pwm_set(PWM_LEFT,  (int32_T)(period * 0.0));
    HAL_Delay(delay);

    pwm_set(PWM_FRONT, (int32_T)(period * 0.0));
    pwm_set(PWM_RIGHT, (int32_T)(period * 0.0));
    pwm_set(PWM_BACK,  (int32_T)(period * 0.0));
    pwm_set(PWM_LEFT,  (int32_T)(period * accelerator_rate / 100.0));
    HAL_Delay(delay);
}

/* 关闭电机 */
void pwm_motor_off(void)
{
    pwm_set(PWM_FRONT, 0);
    pwm_set(PWM_RIGHT, 0);
    pwm_set(PWM_BACK,  0);
    pwm_set(PWM_LEFT,  0);
}

inline int32_T pwm_get_period(void)
{
    return s_period;
}

void pwm_update(const int32_T *val)
{ 
    int32_T i = 0;

    for(i = 0; i < PWM_MAX; i++)
    { 
        pwm_set((PWM_NAME)i, val[i]);
    }
}

