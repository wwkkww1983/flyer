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
#include "misc.h"
#include "data.h"
#include "led.h"
#include "console.h"
#include "pwm.h"
#include "imu.h"
#include "mpu9250.h"
#include "exti.h"
#include "main.h"
#include "inv_mpu.h"
#include "fusion.h"
#include "pid.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "board.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/ 

/********************************** 函数声明区 *********************************/
static void hardware_init(void);
static void hardware_test(void);
static void function_init(void);

static void exti_test(void *argv);

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
    hardware_init();
#define _HARDWARE_DEBUG_
#ifdef _HARDWARE_DEBUG_
    hardware_test();

#else
    function_init();
    while(1)
    { 
        fusion();
        pid();
    }
#endif
}

/* 硬件初始化 */
static void hardware_init(void)
{ 
    /* 关闭心跳中触发imu模块读取 */
    imu_stop(); 
    
    /* step1: 初始化硬件 */
    if(HAL_OK != HAL_Init())
    {
        while(1);
    }

    /* HAL_Init 执行后才可以使用 */
    /* 时钟配置 180M */
    clock_init();

    /* 设置核心中断优先级 */
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, MEM_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(BusFault_IRQn, BUS_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(UsageFault_IRQn, USAGE_INT_PRIORITY, 0);
    HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);

    /* 逐个初始化硬件 */
    /* 串口 */
    console_init(); /* 此后可以开始打印 */ 
    debug_log("串口初始化完成.\r\n");

    /* led */
    led_init();
    debug_log("led初始化完成.\r\n");

    /* pwm */
    pwm_init();
    debug_log("pwm初始化完成.\r\n");

    /* imu i2c */
    imu_init();
    debug_log("imu i2c 初始化完成.\r\n");
}

/* 功能初始化 */
static void function_init(void)
{ 
    /* imu mpu9250 */
    mpu9250_init();
    debug_log("mpu9250 初始化完成.\r\n");

    /* 参数设置 */
    data_config();
    debug_log("参数设置完成.\r\n");

    /* 融合算法初始化 */
    debug_log("融合算法初始化完成.\r\n");
    fusion_init();

    /* step2: 启动心跳中触发imu模块读取 */
    debug_log("imu i2c中断读启动.\r\n");
    imu_start(); /* TODO:中断读还是有BUG 性能问题? */
    
    /* 计算一个融合周期(默认10ms) 融合算法需要时间 */
    /* 测试融合性能达标 */
    debug_log("融合算法测试启动启动.\r\n");
    fusion_test_a_fusion_period();

    /* step3: 启动任务 */
}

/* 硬件测试 */
static void hardware_test(void)
{
    /* 串口 */
    debug_log("串口有打印当然就是好的.\r\n");
    debug_log("可以测试一下串口读取.\r\n");

    /* led */
    debug_log("led测试.\r\n");
    led_toggle(1);
    led_toggle(2);
    led_toggle(3);
    led_toggle(4);
    HAL_Delay(200);
    led_toggle(1);
    led_toggle(2);
    led_toggle(3);
    led_toggle(4);

    /* pwm */
    debug_log("pwm测试.\r\n");
    debug_log("pwm已经有输出,可以修改占空比后测试.\r\n");

    /* int*/
    exti_set_callback(exti_test, "123");
    debug_log("中断测试,请使用 PC13上升沿 触发中断.\r\n");
}

static void exti_test(void *argv)
{
    debug_log("进入中断,参数%s.\r\n", (const char *)argv);
}

