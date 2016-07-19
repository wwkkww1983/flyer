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
#include "math.h"
#include "stm32f4xx_hal.h"
#include "pwm.h"
#include "console.h"
#include "lib_math.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
PWM_LIST_T g_pwm_list[] = {
    {
        .name = PWM_FRONT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_1,
        .val = 0
    },
    {
        .name = PWM_RIGHT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_2,
        .val = 0
    },
    {
        .name = PWM_BACK,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_3,
        .val = 0
    },
    {
        .name = PWM_LEFT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_4,
        .val = 0
    }
};

/* 45度旋转的四元数表示 */
f32_T s_q45[4] = {0.0f};

/* tim句柄 */
static TIM_HandleTypeDef s_tim_handle;
/* pwm_init和pwm_set中都使用 */
static TIM_OC_InitTypeDef s_sConfig;
/* PWM一次脉冲的周期 */
uint32_T s_period = 0;
/********************************** 函数声明区 *********************************/
static uint32_T pwm_get_period(void);

/********************************** 函数实现区 *********************************/

void pwm_init(void)
{
    uint32_T pre_scale_val = 0;
    int32_T i = 0;

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
    for(i = 0; i < PWM_MAX; i++)
    { 
        /* 
         * s_sConfig.Pulse == val 
         * 0 不输出高电平 电机停转
         *
         * */
        pwm_set((PWM_NAME)i, 0);
    }

    f32_T theta = MATH_PI / 4;
    /* 求偏航角旋转45度(绕Z轴)的四元数表示 */
    s_q45[0] = cos(theta / 2);
    s_q45[1] = 0;
    s_q45[2] = 0;
    s_q45[3] = sin(theta / 2);

    return;
}

void pwm_set(PWM_NAME pwm, uint32_T val)
{
    uint32_T period = 0;

    /* 参数检查 */
    if(pwm > PWM_MAX)
    {
        while(1);
    }

    /* 最大占空比为1 */
    period = pwm_get_period();
    if(val > period)
    {
        val = period;
    }

    /* 修改占空比 */
    s_sConfig.Pulse = val;
    if (HAL_TIM_PWM_ConfigChannel(&s_tim_handle, &s_sConfig, g_pwm_list[pwm].ch) != HAL_OK)
    {
        while(1);
    } 
    /* 启动PWM */
    if (HAL_TIM_PWM_Start(&s_tim_handle, g_pwm_list[pwm].ch) != HAL_OK)
    {
        while(1);
    }

}

static uint32_T pwm_get_period(void)
{
    return s_period;
}

void pwm_test(void)
{
    uint32_T m = 0;

    debug_log("前右后左四个pwm分别为 0%% 33%% 77%% 100%%.\r\n");

    m = pwm_get_period();
    m /= 100;

    pwm_set(PWM_FRONT, m * 0);
    pwm_set(PWM_RIGHT, m * 33);
    pwm_set(PWM_BACK, m * 77);
    pwm_set(PWM_LEFT, m * 100);

} 

/* 动力控制 */
void pwm_update(f32_T *q)
{
    f32_T e[3] = {0.0f};
    f32_T q_rotated[4] = {0.0f};

    /* FIXME:可能有问题 两边都需要乘 */
    math_quaternion_cross(q_rotated, q, s_q45); /* 偏航角旋转45度与机翼对应 */
    math_quaternion2euler(e, q_rotated);

    debug_log("横滚角:%7.4f, 俯仰角:%7.4f, 偏航角:%7.4f\r\n", 
            math_arc2angle(e[0]), math_arc2angle(e[1]), math_arc2angle(e[2]));

    /* 实现控制 */
    /* 俯仰角 + 前减后加 */

    /* 俯仰角 - 前加后减 */

    /* 横滚角 + 左减右加 */

    /* 横滚角 - 左加右减 */

}

