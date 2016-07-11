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
#include "misc.h"
#include "led.h"
#include "pwm.h"
#include "console.h"
#include "sensor.h"
#include "esp8266.h"
#include "fusion.h"
#include "lib_math.h"

#include "inv_mpu.h"
#include "mpu9250.h"
#include "inv_mpu_dmp_motion_driver.h"

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

    /* 实际运行 */
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

    static misc_time_T last_loop_start_time;
    static misc_time_T now_time;
    static misc_time_T last_interval;
    static misc_time_T interval;
    static misc_time_T temp;

    f32_T q[4] = {0.0f};
    f32_T e[3] = {0.0f};

    if(TRUE == first_run) /* 获取起始时间 仅运行一次 */
    {
        ms_start = HAL_GetTick();
        first_run = FALSE;
    } 
    else
    {
        /* 第一次不运行 其他每次都运行 */
        /* 冒泡算法 interval中永远存放 最大间隔 */
        get_now(&now_time);
        diff_clk(&interval, &last_loop_start_time, &now_time);

        if(diff_clk(&temp, &interval, &last_interval))
        {
            last_interval.ms = interval.ms;
            last_interval.clk = interval.clk;
        }
    }
    
    ms_end = HAL_GetTick();
    if(ms_end - ms_start >= 2500) /* 已达2.5s 执行一次 */
    {
        /* 该处代码 每秒执行一次 */
        led_toggle(LED_MLED);
        ms_start = HAL_GetTick();
        console_printf("%4.1f秒:", ms_start / 1000.0f); 
        get_quaternion(q);
        math_quaternion2euler(e, q);
        console_printf("姿态:%.4f, %.4f, %.4f <==> %.4f,%.4f,%.4f,%.4f\r\n", math_arc2angle(e[0]), math_arc2angle(e[1]), math_arc2angle(e[2]),
                q[0], q[1], q[2], q[3]);
        console_printf("主循环最大耗时:%ums,%5.2fus.\r\n", last_interval.ms, 1.0f * last_interval.clk / 84);
    } 

    get_now(&last_loop_start_time);
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
    //console_printf("自检完成.\r\n");

    console_printf("系统初始化完成.\r\n");
}

/* 硬件测试 */
//static uint8_T c = 0;
//
static int32_T rst = 0;
static uint8_T int_cfg = 0;
static uint8_T int_en = 0;
static uint8_T int_sta = 0;
static int16_T s_gyro[3] = {0};
static int16_T s_accel_short[3] = {0};
static int16_T s_sensors = 0;
static uint8_T s_more = 0;
static int32_T s_quat[4] = {0};
static int32_T s_temperature = 0;
static void self_test(void)
{
    TRACE_FUNC_IN; 
    console_printf("开始硬件测试.\r\n"); 

    while(1)
    {
        if(g_pp_fifo_ready) 
        {
            int32_T i = 0;

            g_pp_fifo_ready = FALSE; 
            rst = mpu_read_reg(0x37, &int_cfg);
            rst = mpu_read_reg(0x38, &int_en);
            rst = mpu_read_reg(0x3A, &int_sta);
            rst = dmp_read_fifo(s_gyro, s_accel_short,
                    (long *)s_quat, (unsigned long *)&s_temperature, &s_sensors, &s_more);

            if (!s_more)
            {
                i = 1;
            }

            if (s_sensors & INV_XYZ_GYRO)
            {
                i = 2;
            }

            if (s_sensors & INV_XYZ_ACCEL) 
            {
                i = 3;
            }

            if (s_sensors & INV_WXYZ_QUAT) 
            {
                i = 4;
            }

            UNUSED(rst);
            UNUSED(i);
        }
    }

    console_test();

    led_test();

    pwm_test();

    sensor_test();

    esp8266_test();
    
    fusion_test();

    console_printf("结束硬件测试.\r\n"); 
    TRACE_FUNC_OUT;
}

