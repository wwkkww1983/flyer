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
#include "led.h"
#include "pwm.h"
#include "console.h"
#include "sensor.h"
#include "esp8266.h"
#include "fusion.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/ 

/********************************** 函数声明区 *********************************/
static void idle(void);

/*******************************************************************************
*
* 函数名  : init
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: 系统初始化
*
* 输入参数: 无
* 输出参数: 无
* 返回值  : 无
*
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
static void init(void);

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
* 调用关系: 贴片完成后测试板子是否正常工作
* 其 它   : 无
*
******************************************************************************/
static void self_test(void);

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
    init();

    console_printf("\r\n开始进入主循环.\r\n");
    while(1)
    { 
        /* 采样 */
        sensor_read();
        /* 融合 */
        fusion();
        /* 动力控制 */
        pwm_update();
        /* 以上实时性要求强 否则坠机 */

        /* 以下实时性要求不强  */
        /* 处理交互 */
        esp8266_task();
        /* 收尾统计工作 */
        idle();
    }
}

/* 每秒周期性执行 */
static void idle(void)
{
    static bool_T first_run = TRUE;
    static uint32_T ms_start = 0;
    static uint32_T ms_end = 0;

    static uint32_T this_interval = 0;
    static uint32_T interval = 0;

    if(TRUE == first_run) /* 获取起始时间 仅运行一次 */
    {
        ms_start = HAL_GetTick();
        first_run = FALSE;
    } 
    else
    {
        /* 第一次不运行 其他每次都运行 */
        /* 冒泡算法 interval中永远存放 最大间隔 */
        this_interval = HAL_GetTick() - ms_end;
        if(interval < this_interval)
        {
            interval = this_interval;
            console_printf("间隔增大到 %ums.\r\n", interval);
        }
    }
    
    ms_end = HAL_GetTick();
    if(ms_end - ms_start >= 1000) /* 已达1s */
    {
        /* 该处代码 每秒执行一次 */
        led_toggle(LED_MLED);
        ms_start = HAL_GetTick();
        //console_printf("%ums:我还在.\r\n", ms_start);
    }


}

/* 初始化 */
static void init(void)
{ 
    /* step1: hal初始化 */
    if(HAL_OK != HAL_Init())
    {
        while(1);
    }

    /* step2: 配置时钟 HAL_Init 执行后才可执行 */
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
    console_printf("console初始化完成.\r\n");
    console_printf("系统时钟频率:%dMHz\r\n", SystemCoreClock / 1000 / 1000);
		
    /* led */
    led_init();
    console_printf("led初始化完成.\r\n");

    /* pwm */
    pwm_init();
    console_printf("pwm初始化完成.\r\n"); 

    /* 姿态传感器 */
    sensor_init();
    console_printf("sensor(MPU9250+AK8963+BMP280)初始化完成.\r\n");

    /* wifi 模块串口 */
    esp8266_init();
    console_printf("esp8266 wifi模块初始化完成.\r\n");

    /* 姿态融合算法 */
    fusion_init();
    console_printf("姿态融合算法初始化完成.\r\n");

    /* 自检 */
    self_test();
    console_printf("自检完成.\r\n");

    console_printf("系统初始化完成.\r\n");
}

/* 硬件测试 */
//static uint8_T c = 0;
static void self_test(void)
{
    TRACE_FUNC_IN; 
    console_printf("开始硬件测试.\r\n"); 

    console_test();

    led_test();

    pwm_test();

    sensor_test();

    esp8266_test();
    
    fusion_test();

    console_printf("结束硬件测试.\r\n"); 
    TRACE_FUNC_OUT;
}

