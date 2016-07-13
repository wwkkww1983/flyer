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
#include "misc.h"
#include "led.h"
#include "pwm.h"
#include "console.h"
#include "sensor.h"
#include "esp8266.h"
#include "fusion.h"
#include "lib_math.h"

#include "si.h"
#include "inv_mpu.h"
#include "mpu9250.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "uart.h"
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
//static void self_test(void);

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
    debug_log("\r\n开始进入主循环.\r\n");

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
        debug_log("%4.1f秒:", ms_start / 1000.0f); 
        get_quaternion(q);
        math_quaternion2euler(e, q);
        debug_log("姿态:%.4f, %.4f, %.4f <==> %.4f,%.4f,%.4f,%.4f\r\n",
                math_arc2angle(e[0]), math_arc2angle(e[1]), math_arc2angle(e[2]),
                q[0], q[1], q[2], q[3]);
        debug_log("主循环最大耗时:%ums,%5.2fus.\r\n",
                last_interval.ms, 1.0f * last_interval.clk / 84);
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
    debug_log("console初始化完成.\r\n");
    debug_log("系统时钟频率:%dMHz\r\n", SystemCoreClock / 1000 / 1000);
		
    /* led */
    led_init();
    debug_log("led初始化完成.\r\n");

    /* pwm */
    pwm_init();
    debug_log("pwm初始化完成.\r\n"); 

    /* 姿态传感器 */
    sensor_init();
    debug_log("sensor(MPU9250+AK8963+BMP280)初始化完成.\r\n");

    /* wifi 模块串口 */
    esp8266_init();
    debug_log("esp8266 wifi模块初始化完成.\r\n");

    /* 姿态融合算法 */
    fusion_init();
    debug_log("姿态融合算法初始化完成.\r\n");

#if 1
    static void hard_fusion(void);
    hard_fusion();

#else

    /* 自检 */
    self_test();
    debug_log("自检完成.\r\n");

    debug_log("系统初始化完成.\r\n");
#endif
}

#define FRAME_LENGTH        (32U) 
static int32_T times = 0; 
static misc_time_T time1, time2;
static misc_time_T diff1;
static void hard_fusion(void)
{
    /* 硬解 */
    static int32_T rst = 0;
    static uint8_T ak8963_buf[AK8963_DATA_LENGTH] = {0};
    static uint8_T mpuu9250_buf[MPU9250_ATG_LENGTH] = {0};
    static uint8_T frame[FRAME_LENGTH] = {0};
    static uint8_T send_buf[FRAME_LENGTH] = {0};
    static uint32_T tick = 0;

    static int32_T crc32 = 0;
    /* 硬件crc */
    static CRC_HandleTypeDef crc;
    
    /* 长度 */
    frame[0] = 0;
    frame[1] = 30;

    crc.Instance = CRC;
		__HAL_RCC_CRC_CLK_ENABLE();
    if(HAL_OK != HAL_CRC_Init(&crc))
    {
        while(1);
    }

    /* 填充 */
    frame[2] = 0x00;
    frame[3] = 0x00;

#if 0
    /* 标记 */
    frame[4] = 0x00;
    frame[5] = 0x00;

    /* time */
    frame[6] = 0;
    frame[7] = 0;
    frame[8] = 0;
    frame[9] = 0;

    /* 加计 */
    frame[10] = 0x00;
    frame[11] = 0x00;
    frame[12] = 0x00;
    frame[13] = 0x00;
    frame[14] = 0x00;
    frame[15] = 0x00;

    /* 陀螺仪 */
    frame[16] = 0x00;
    frame[17] = 0x00;
    frame[18] = 0x00;
    frame[19] = 0x00;
    frame[20] = 0x00;
    frame[21] = 0x00;

    /* 磁力计 */
    frame[22] = 0x00;
    frame[23] = 0x00;
    frame[24] = 0x00;
    frame[25] = 0x00;
    frame[26] = 0x00;
    frame[27] = 0x00;

    /* crc32 */
    frame[28] = 0x00;
    frame[29] = 0x00;
    frame[30] = 0x00;
    frame[31] = 0x00;
#endif

    console_printf("data:\r\n");
		HAL_Delay(1000);
    while(1)
    { 
        frame[7] = 0x00;
#if 1
        if(1 == times)
        {
            get_now(&time1);
        }
        if(2 == times)
        {
            get_now(&time2);
            diff_clk(&diff1, &time1, &time2);
        }
#endif

        tick = HAL_GetTick();
        frame[6] = (uint8_T)(tick >> 24);
        frame[7] = (uint8_T)((tick >> 16) & 0xff);
        frame[8] = (uint8_T)((tick >> 8) & 0xff);
        frame[9] = (uint8_T)((tick) & 0xff);
        if(0 == tick % 5) /* 大约 5ms true一次 */
        {
#if 0
            get_now(&time1);
#endif

            mpu9250_read(ACCEL_TYPE | GYRO_TYPE, mpuu9250_buf); /* dma读mpu9250数据 */
            while(!si_read_ready()); /* 等待读完成 */

            frame[5] |= 0x02; /* 加计 */
            frame[10] = mpuu9250_buf[0];
            frame[11] = mpuu9250_buf[1];
            frame[12] = mpuu9250_buf[2];
            frame[13] = mpuu9250_buf[3];
            frame[14] = mpuu9250_buf[4];
            frame[15] = mpuu9250_buf[5];

            frame[5] |= 0x04; /* 陀螺仪 */
            frame[16] = mpuu9250_buf[8];
            frame[17] = mpuu9250_buf[9];
            frame[18] = mpuu9250_buf[10];
            frame[19] = mpuu9250_buf[11];
            frame[20] = mpuu9250_buf[12];
            frame[21] = mpuu9250_buf[13];

            ak8963_read(ak8963_buf); /* dma读磁力计数据 */
            while(!si_read_ready()); /* 等待读完成 */ 
            /* 填充磁力计数据 */
            if((AK8963_ST1_DRDY_BIT & ak8963_buf[0])   /* 有效数据 */ 
            && !(AK8963_ST2_HOFL_BIT & ak8963_buf[7])) /* 未超量程溢出 */
            { 
                frame[5] |= 0x08; /* 磁力计 */
                frame[22] = ak8963_buf[1];
                frame[23] = ak8963_buf[2];
                frame[24] = ak8963_buf[3];
                frame[25] = ak8963_buf[4];
                frame[26] = ak8963_buf[5];
                frame[27] = ak8963_buf[6];
            } 

            /* 使用crc32校验 */
            crc32 = HAL_CRC_Calculate(&crc, (uint32_T *)frame, FRAME_LENGTH / sizeof(uint32_T) - 1); 
            frame[28] =  (uint8_T)( crc32 >> 24);
            frame[29] =  (uint8_T)((crc32 >> 16) & 0xff);
            frame[30] =  (uint8_T)((crc32 >> 8) & 0xff);
            frame[31] =  (uint8_T)(crc32  & 0xff); 
            
            /* 双缓冲frame/send_buf 避免串口被组帧干扰 */
            /* 串口发送 */
            while(uart_tc_locked(&g_console)); /* 等待上一次传输完成 */
            memcpy(send_buf, frame, FRAME_LENGTH); 
            uart_send_bytes(&g_console, send_buf, FRAME_LENGTH);
#if 0
            get_now(&time2);
            diff_clk(&diff1, &time1, &time2);
            times++;
#endif
            /* 避免1ms中多次读取 */
            HAL_Delay(1);
            UNUSED(rst); 
            UNUSED(times);
        }
    }
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
    sensor_test();
    esp8266_test();    
    fusion_test();

    debug_log("结束硬件测试.\r\n"); 
    TRACE_FUNC_OUT;
}
#endif
