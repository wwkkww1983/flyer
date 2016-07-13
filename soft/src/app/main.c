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

#define FRAME_LENGTH        (48U) 
static int32_T times = 0; 
static misc_time_T time1, time2;
static misc_time_T diff1;
static void hard_fusion(void)
{
    /* 硬解 */
    static int32_T rst = 0;
    static int16_T s_gyro[3] = {0};
    static int16_T s_accel_short[3] = {0};
    static int16_T s_sensors = 0;
    static uint8_T s_more = 0;
    static int32_T s_quat[4] = {0};
    static int32_T s_temperature = 0;
    static uint8_T buf[AK8963_DATA_LENGTH] = {0};
    static uint8_T frame[FRAME_LENGTH+1] = {0};

    static int32_T crc32 = 0;
    /* 硬件crc */
    static CRC_HandleTypeDef crc;
    
    /* 长度定长 */
    frame[0] = 0;
    frame[1] = 46;

    if(HAL_OK != HAL_CRC_Init(&crc))
    {
        while(1);
    }

#if 0
    /* time */
    frame[2] = 42;
    frame[3] = 42;
    frame[4] = 42;
    frame[5] = 42;
#endif

    /* type */
    frame[6] = 0x00;
    frame[7] = 0x00;

#if 0
    /* 四元数 */
    frame[8] = 0x00;
    frame[9] = 0x00;
    frame[10] = 0x00;
    frame[11] = 0x00;
    frame[12] = 0x00;
    frame[13] = 0x00;
    frame[14] = 0x00;
    frame[15] = 0x00;
    frame[16] = 0x00;
    frame[17] = 0x00;
    frame[18] = 0x00;
    frame[19] = 0x00;
    frame[20] = 0x00;
    frame[21] = 0x00;
    frame[22] = 0x00;
    frame[23] = 0x00;

    /* 加计 */
    frame[24] = 0x00;
    frame[25] = 0x00;
    frame[26] = 0x00;
    frame[27] = 0x00;
    frame[28] = 0x00;
    frame[29] = 0x00;

    /* 陀螺仪 */
    frame[30] = 0x00;
    frame[31] = 0x00;
    frame[32] = 0x00;
    frame[33] = 0x00;
    frame[34] = 0x00;
    frame[35] = 0x00;

    /* 磁力计 */
    frame[36] = 0x00;
    frame[37] = 0x00;
    frame[38] = 0x00;
    frame[39] = 0x00;
    frame[40] = 0x00;
    frame[41] = 0x00;
#endif

    /* 填充 */
    frame[42] = 0x00;
    frame[43] = 0x00;

    /* crc32 */
    frame[44] = 0x00;
    frame[45] = 0x00;
    frame[46] = 0x00;
    frame[47] = 0x00;
   
    /* 末尾空字符 */
    frame[48] = NUL;

    while(1)
    { 
        frame[7] = 0x00;
        if(g_mpu9250_fifo_ready) /* 大约 5ms true一次 */
        { 
            get_now(&time1);

            g_mpu9250_fifo_ready = FALSE; 

            ak8963_read(buf); /* dma读磁力计数据 */
            while(!si_read_ready()); /* 等待读完成 */ 
            
            /* 填充磁力计数据 */
            if((AK8963_ST1_DRDY_BIT & buf[0])   /* 有效数据 */ 
            && !(AK8963_ST2_HOFL_BIT & buf[7])) /* 未超量程溢出 */
            { 
                frame[7] |= 0x08; /* 磁力计 */
                frame[36] = buf[1];
                frame[37] = buf[2];
                frame[38] = buf[3];
                frame[39] = buf[4];
                frame[40] = buf[5];
                frame[41] = buf[6];
            }
#if 0 
            static uint8_T int_cfg = 0;
            static uint8_T int_en = 0;
            static uint8_T int_sta = 0;
            rst = mpu_read_reg(0x37, &int_cfg);
            rst = mpu_read_reg(0x38, &int_en);
            rst = mpu_read_reg(0x3A, &int_sta);
#endif      
            /* TODO: 看看读取的时间 查看是否有必要马上优化为DMA读 */
            rst = dmp_read_fifo(s_gyro, s_accel_short,
                    (long *)s_quat, (unsigned long *)&s_temperature, &s_sensors, &s_more); 

            if (s_sensors & INV_XYZ_GYRO)
            {
                frame[7] |= 0x04; /* 陀螺仪 */ 
                frame[30] = (uint8_T)(s_gyro[0] >> 8);
                frame[31] = (uint8_T)(s_gyro[0] & 0xff);
                frame[32] = (uint8_T)(s_gyro[1] >> 8);
                frame[33] = (uint8_T)(s_gyro[1] & 0xff);
                frame[34] = (uint8_T)(s_gyro[2] >> 8);
                frame[35] = (uint8_T)(s_gyro[2] & 0xff);
            }
            if (s_sensors & INV_XYZ_ACCEL) 
            {
                frame[7] |= 0x02; /* 加计 */
                frame[24] = (uint8_T)(s_accel_short[0] >> 8);
                frame[25] = (uint8_T)(s_accel_short[0] & 0xff);
                frame[26] = (uint8_T)(s_accel_short[1] >> 8);
                frame[27] = (uint8_T)(s_accel_short[1] & 0xff);
                frame[28] = (uint8_T)(s_accel_short[2] >> 8);
                frame[29] = (uint8_T)(s_accel_short[2] & 0xff);
            }
            if (s_sensors & INV_WXYZ_QUAT) 
            {
                frame[7] |= 0x01; /* 四元数 */ 
                
                frame[8] =  (uint8_T)( s_quat[0] >> 24);
                frame[9] =  (uint8_T)((s_quat[0] >> 16) & 0xff);
                frame[10] = (uint8_T)((s_quat[0] >> 8) & 0xff);
                frame[11] = (uint8_T)( s_quat[0] & 0xff);
                frame[12] = (uint8_T)( s_quat[1] >> 24);
                frame[13] = (uint8_T)((s_quat[1] >> 16) & 0xff);
                frame[14] = (uint8_T)((s_quat[1] >> 8) & 0xff);
                frame[15] = (uint8_T)( s_quat[1] & 0xff);
                frame[16] = (uint8_T)( s_quat[2] >> 24);
                frame[17] = (uint8_T)((s_quat[2] >> 16) & 0xff);
                frame[18] = (uint8_T)((s_quat[2] >> 8) & 0xff);
                frame[19] = (uint8_T)( s_quat[2] & 0xff);
                frame[20] = (uint8_T)( s_quat[3] >> 24);
                frame[21] = (uint8_T)((s_quat[3] >> 16) & 0xff);
                frame[22] = (uint8_T)((s_quat[3] >> 8) & 0xff);
                frame[23] = (uint8_T)( s_quat[3] & 0xff);

#if 0
                q[0] = (f32_T) s_quat[0] / ((f32_T)(1L << 30));
                q[1] = (f32_T) s_quat[1] / ((f32_T)(1L << 30));
                q[2] = (f32_T) s_quat[2] / ((f32_T)(1L << 30));
                q[3] = (f32_T) s_quat[3] / ((f32_T)(1L << 30)); 
#endif
            }

            /* 使用crc32校验 */
            crc32 = HAL_CRC_Calculate(&crc, (uint32_T *)frame, FRAME_LENGTH / sizeof(uint32_T)); 
            frame[44] =  (uint8_T)( crc32 >> 24);
            frame[45] =  (uint8_T)((crc32 >> 16) & 0xff);
            frame[46] =  (uint8_T)((crc32 >> 8) & 0xff);
            frame[47] =  (uint8_T)(crc32  & 0xff);

            /* 串口发送 */
            console_printf(frame);

            get_now(&time2);
            diff_clk(&diff1, &time1, &time2);

            UNUSED(rst); 
            times++;
        }

        if(0 == times)
        {
            get_now(&time1);
        }
        if(1 == times)
        {
            get_now(&time2);
            diff_clk(&diff1, &time1, &time2);
        }
    }
}

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

