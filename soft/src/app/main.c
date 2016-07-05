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
#include "esp8266.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/ 

/********************************** 函数声明区 *********************************/
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
* 函数名  : hard_test
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: 验证硬件是否正常
*
* 输入参数: 无
* 输出参数: 无
* 返回值  : 无
*
* 调用关系: 贴片完成后测试板子硬件是否正常工作
* 其 它   : 无
*
******************************************************************************/
static void hard_test(void);

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
    hard_test();

    while(1);
}

/* 初始化 */
static void init(void)
{ 

    /* step1: hal初始化 */
    if(HAL_OK != HAL_Init())
    {
        while(1);
    }
		
    /*Configure the SysTick to have interrupt in 1ms time basis*/
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/TICK_PER_SECONDS);

    /* step2: 配置时钟 HAL_Init 执行后才可执行 */
    /* 时钟配置 84M */
    clock_init();

    /* 设置核心中断优先级 */
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, MEM_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(BusFault_IRQn, BUS_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(UsageFault_IRQn, USAGE_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);

    /* 逐个初始化硬件 */
    /* 控制台串口 */
    console_init(); /* 此后可以开始打印 */ 
    console_printf("console初始化完成.\r\n");

    /* led */
    led_init();
    console_printf("led初始化完成.\r\n");

    /* pwm */
    pwm_init();
    console_printf("pwm初始化完成.\r\n"); 

#if 0
    /* i2c */
    imu_init()
    debug_log("MPU9250+BMP280初始化完成.\r\n");
#endif

    /* wifi 模块串口 */
    esp8266_init();
    esp8266_printf("flyer初始化完成.\r\n");
    console_printf("esp8266 wifi模块初始化完成.\r\n");
		
    console_printf("初始化完成.\r\n");
}

/* 硬件测试 */
//static uint8_T c = 0;
static void hard_test(void)
{
    TRACE_FUNC_IN; 

    console_printf("观察控制台输出并按照提示操作.\r\n");
    console_test();

    console_printf("观察led是否有闪烁.\r\n"); 
    led_test();

    console_printf("四个pwm分别为 1/4 2/4 3/4 4/4.\r\n"); 
    pwm_test();

    console_printf("观察esp8266并按照提示操作.\r\n");
    esp8266_test();

    console_printf("结束硬件测试.\r\n"); 

    TRACE_FUNC_OUT;
}

