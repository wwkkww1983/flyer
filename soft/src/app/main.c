/******************************************************************************
 *
 * 文件名  ： main.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160112
 * 版本号  ： 1.2
 * 文件描述： 飞控主控模块 不使用os
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无 
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
/* 消除中文打印警告 */
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include "config.h"
#include "board.h"
#include <stm32f4xx_hal.h>
#include <math.h>
#include "misc.h"
#include "led.h"
#include "pwm.h"
#include "si.h"
#include "mpu9250.h"
#include "console.h"
#include "esp8266.h"
#include "comm.h"
#include "lib_math.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/ 
f32_T s_q_rotated[4] = {0.0f}; /* 旋转后的与机翼方向一致的四元数 */

/********************************** 函数声明区 *********************************/
static void idle(void);
static void init(void);

#if 0
/*******************************************************************************
*
* 函数名  : self_test
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: 自检验证设备是否正常
*
* 输入参数: 无
* 输出参数: 无
* 返回值  : 无
*
* 调用关系: 测试硬件是否正常工作
* 其 它   : 无
*
******************************************************************************/
static void self_test(void);
#endif

/********************************** 函数实现区 *********************************/
/*******************************************************************************
*
* 函数名  : main
* 负责人  : 彭鹏
* 创建日期: 20160112
* 函数功能: MPU9250 主函数
*
* 输入参数: 无
* 输出参数: 无
* 返回值  : 主程序永不返回
*
* 调用关系: 无
* 其 它   : 获取MPU9250数据 中断中完成
*
******************************************************************************/
int main(void)
{ 
    f32_T quat[4] = {0.0f}; /* mpu9250 dmp四元数 */
    f32_T q45[4] = {0.0f}; /* 求偏航角旋转45度pi/4(绕Z轴)的四元数表示 */
    f32_T theta = MATH_PI / 4;

    q45[0] = cos(theta / 2);
    q45[1] = 0;
    q45[2] = 0;
    q45[3] = sin(theta / 2);

    init();
    debug_log("\r\n开始进入主循环.\r\n");

    /* 实际运行 */
    while(1)
    { 
        /* 采样 */
        mpu9250_dmp_read(quat); 
        /* 偏航角旋转45度与机翼对应 */
        /* FIXME:可能有问题 两边都需要乘 */
        math_quaternion_cross(s_q_rotated, quat, q45);
        /* 动力控制 */
        pwm_update(s_q_rotated);
        /* 以上实时性要求强 否则坠机 */

        /* 以下实时性要求不强  */
        /* 处理交互 */
        comm_task();
        /* 收尾统计工作 */
        idle();
    }
}

/* 每秒周期性执行 */
static void idle()
{
    static bool_T first_run = TRUE; /* 标记是否首次运行 */
    static uint32_T ms_start = 0; /* 首次运行ms数*/
    static misc_time_T last_loop_start_time = {0}; /* 上次主循环启动时间 */
    static misc_time_T max_interval = {0}; /* 存放每次主循环最大耗时(用于评估时间片) */

    misc_time_T interval = {0};
    uint32_T ms_now = 0;
    misc_time_T now_time = {0};
    misc_time_T temp = {0};

    if(TRUE == first_run) /* 获取起始时间 仅运行一次 */
    {
        ms_start = HAL_GetTick();
        first_run = FALSE;
    } 
    else /* 第一次不运行 其他每次都运行 */
    {
        /* (本次)主循环终点 */
        get_now(&now_time);
        diff_clk(&interval, &last_loop_start_time, &now_time);

        /* 冒泡算法max_interval中永远存放 最大间隔 */
        if(1 == diff_clk(&temp, &interval, &max_interval))
        {
            max_interval.ms = interval.ms;
            max_interval.clk = interval.clk;
        }

        ms_now = HAL_GetTick();
        if(ms_now - ms_start >= 1000) /* 已达2.5s 执行一次 */
        {
            /* 该处代码 每秒执行一次 */
            led_toggle(LED_MLED);
            ms_start = HAL_GetTick();

            debug_log("%4.1f秒:", ms_start / 1000.0f);
            debug_log("loop max time:%dms,%5.2fus.\r\n", 
                    max_interval.ms, 1.0f * max_interval.clk / 84);
        }

    }

    /* (下次)主循环起点 */
    get_now(&last_loop_start_time);
}

/* 初始化 */
static void init(void)
{ 
    /* hal初始化 */
    if(HAL_OK != HAL_Init())
    {
        while(1);
    }

    /* 配置时钟 HAL_Init 执行后才可执行 */
    /* 时钟配置 84MHz */
    clock_init();

    /* 配置中断频率为 1ms
     * systick时钟为系统AHB时钟:84MHz
     * 注意: clock_init会改变SystemCoreClock值
     * */
    if(0 != HAL_SYSTICK_Config(SystemCoreClock / TICK_PER_SECONDS))
    {
        while(1);
    }

    /* 设置核心中断优先级 */
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, MEM_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(BusFault_IRQn, BUS_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(UsageFault_IRQn, USAGE_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);

    /* 逐个初始化硬件 */
    /* 控制台串口 */
    console_init(); /* 此后可以开始打印 */
    debug_log("console初始化完成.\r\n");
    debug_log("系统时钟频率:%dMHz\r\n", SystemCoreClock / 1000 / 1000);

    /* wifi 模块串口 */
    esp8266_init();
    debug_log("esp8266 wifi模块初始化完成.\r\n");

    /* 配置交互协议模块(必须等待console和esp8266初始化完成) */
    comm_init(&g_console);
    debug_log("交互模块初始化完成.\r\n");

    /* led */
    led_init();
    debug_log("led初始化完成.\r\n");

    /* pwm */
    pwm_init();
    debug_log("pwm初始化完成.\r\n"); 

    /* 姿态传感器i2c总线初始化 */
    si_init();
    debug_log("传感器i2c总线 初始化完成.\r\n");

    /* 姿态传感器 */
    mpu9250_init();
    debug_log("MPU9250初始化完成.\r\n");

#if 0
    /* 自检 */
    self_test();
    debug_log("自检完成.\r\n");
#endif

    debug_log("系统初始化完成.\r\n");
}

/* FIXME:考虑同步问题 */
void get_quat(f32_T *q)
{
    q[0] = s_q_rotated[0];
    q[1] = s_q_rotated[1];
    q[2] = s_q_rotated[2];
    q[3] = s_q_rotated[3];
}

#if 0
/* 硬件测试 */
static void self_test(void)
{
    TRACE_FUNC_IN; 
    debug_log("开始硬件测试.\r\n"); 

    console_test();
    led_test();
    pwm_test();
    mpu9250_test();
    esp8266_test();    

    debug_log("结束硬件测试.\r\n"); 
    TRACE_FUNC_OUT;
}
#endif
