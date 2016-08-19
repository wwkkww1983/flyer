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
/* base域表示基础值(用于运动) pwm_update中计算矫正值(用于平衡) */
PWM_LIST_T g_pwm_list[] = {
    {
        .name = PWM_FRONT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_1,
        .base = 0,
        .adj_val  = 0,
        .adj_step = 0
    },
    {
        .name = PWM_RIGHT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_2,
        .base = 0,
        .adj_val  = 0,
        .adj_step = 0
    },
    {
        .name = PWM_BACK,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_3,
        .base = 0,
        .adj_val  = 0,
        .adj_step = 0
    },
    {
        .name = PWM_LEFT,
        .tim  = PWM_TIM, 
        .ch   = TIM_CHANNEL_4,
        .base = 0,
        .adj_val  = 0,
        .adj_step = 0
    }
};

/* 45度旋转的四元数表示 */
f32_T s_q45[4] = {0.0f};

/* tim句柄 */
static TIM_HandleTypeDef s_tim_handle;
/* pwm_init和pwm_set中都使用 */
static TIM_OC_InitTypeDef s_sConfig;
/* PWM一次脉冲的周期 */
static int32_T s_period = 0;
/********************************** 函数声明区 *********************************/
static void pwm_set(PWM_NAME pwm, int32_T val);

/********************************** 函数实现区 *********************************/

void pwm_init(void)
{
    uint32_T pre_scale_val = 0;
    int32_T i = 0;

    /* 数据结构初始化 */
    for(i = 0; i < PWM_MAX; i++)
    { 
        g_pwm_list[i].base = 0;
        g_pwm_list[i].adj_val = 0;
        g_pwm_list[i].adj_step = 1; /* 初始步长定小 */
    }

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

    return;
}

static void pwm_set(PWM_NAME pwm, int32_T val)
{
    uint32_T period = 0;

    /* 参数检查 */
    if(pwm > PWM_MAX)
    {
        while(1);
    }

    /* val >= 0 */
    if(val < 0)
    {
        val = 0;
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

inline int32_T pwm_get_period(void)
{
    return s_period;
}

inline int32_T pwm_get_step(void)
{
    int32_T period = 0;

    period = pwm_get_period();

    return period / 100;
}

void pwm_test(void)
{
    int32_T period = 0;

    debug_log("前右后左四个pwm分别为 0%% 33%% 77%% 100%%.\r\n");

    period = pwm_get_period();

    pwm_set(PWM_FRONT, (int32_T)(period * 0.0));
    pwm_set(PWM_RIGHT, (int32_T)(period * 0.33));
    pwm_set(PWM_BACK,  (int32_T)(period * 0.77));
    pwm_set(PWM_LEFT,  (int32_T)(period * 1.0));

} 

/* 平衡控制 */
void pwm_update(void)
{
    f32_T e[3] = {0.0f};
    f32_T q[4] = {0.0f}; 
    f32_T theta = 0.0f;
    f32_T psi = 0.0f;
    /* f32_T phi = 0.0f; */
    int32_T val[PWM_MAX] = {0};
    int32_T period = 0;

    period = pwm_get_period();

    mpu9250_get_quat(q);
    math_quaternion2euler(e, q);

    theta = e[0];
    psi = e[1];

    /* 计算矫正值 */
    /* 俯仰角 + 前减后加 */
    if(theta > 0)
    {
        g_pwm_list[PWM_FRONT].adj_val += g_pwm_list[PWM_FRONT].adj_step;
        g_pwm_list[PWM_BACK].adj_val -= g_pwm_list[PWM_BACK].adj_step;
    }
    /* 俯仰角 - 前加后减 */
    else if(theta < 0)
    {
        g_pwm_list[PWM_FRONT].adj_val -= g_pwm_list[PWM_FRONT].adj_step;
        g_pwm_list[PWM_BACK].adj_val += g_pwm_list[PWM_BACK].adj_step;
    }
    else
    {
        ;
    }

    /* 横滚角 + 左减右加 */
    if(psi > 0)
    {
        g_pwm_list[PWM_LEFT].adj_val += g_pwm_list[PWM_LEFT].adj_step;
        g_pwm_list[PWM_RIGHT].adj_val -= g_pwm_list[PWM_RIGHT].adj_step;
    }
    /* 横滚角 - 左加右减 */
    else if(psi < 0)
    {
        g_pwm_list[PWM_LEFT].adj_val -= g_pwm_list[PWM_LEFT].adj_step;
        g_pwm_list[PWM_RIGHT].adj_val += g_pwm_list[PWM_RIGHT].adj_step;
    }
    else
    {
        ;
    } 
    
    /* 限幅 */
    for(int32_T i = 0; i < PWM_MAX; i++)
    {
        if(g_pwm_list[i].adj_val < - period * PWM_ADJ_MAX_RATE)
        { 
            g_pwm_list[i].adj_val = - period * PWM_ADJ_MAX_RATE;
        }

        if(g_pwm_list[i].adj_val > period * PWM_ADJ_MAX_RATE)
        {
            g_pwm_list[i].adj_val = period * PWM_ADJ_MAX_RATE;
        }
    }
    
    pwm_get_acceleralor(val);

    for(int32_T i = 0; i < PWM_MAX; i++)
    { 
        pwm_set((PWM_NAME)i, val[i]);
    }
} 

void pwm_set_acceleralor(const int32_T *val_list)
{
    int32_T val = 0;
    int32_T period = 0;
    period = pwm_get_period(); 
    
    for(int32_T i = 0; i < PWM_MAX; i++)
    { 
        val = val_list[i];
        /* 限幅 */
        if(val < 0)
        {
            val = 0;
        }
        if(val > period)
        {
            val = period;
        } 
        
        g_pwm_list[i].base = val;
    }

}

void pwm_get_acceleralor(int32_T *val)
{ 
    int32_T adj_val = 0;
    int32_T base = 0;
    int32_T period = 0;

    period = pwm_get_period();

    for(int32_T i = 0; i < PWM_MAX; i++)
    {
        adj_val = g_pwm_list[i].adj_val;
        base = g_pwm_list[i].base;
        val[i] = base + adj_val;

        /* 限幅 */
        if(val[i] < 0)
        {
            val[i] = 0;
        }
        if(val[i] > period)
        {
            val[i] = period;
        }
    }
}

