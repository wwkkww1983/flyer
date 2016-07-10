/******************************************************************************
 *
 * 文件名  ： sensor接口
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160112
 * 版本号  ： 1.0
 * 文件描述： sensor共有代码
 * 版权说明： Copyright (c) GNU
 * 其    他： 实现非阻塞
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
/* 消除中文打印警告 */
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"
#include "misc.h"
#include "si.h"
#include "inv_mpu.h"
#include "mpu9250.h"
#include "ak8963.h"
#include "bmp280_hal.h"
#include "console.h"
#include "sensor.h"
/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数实现区 *********************************/
void sensor_init(void)
{
    /* mpu9250初始化 */
    mpu9250_init();
    console_printf("mpu9250 初始化完成.\r\n");

    /* ak8963 初始化 */
    ak8963_init();
    ak8963_sample_period(10);
    console_printf("mpu9250 初始化完成.\r\n");

    bmp280_hal_init();
    console_printf("bmp280_init 初始化完成.\r\n");

    return;
} 

/* 获取校准参数 */
void sensor_get_sens(uint16_T *accel_sens, f32_T *gyro_sens, int16_T *mag_sens)
{
    /* 使用inv_mpu.c */ 
    if(NULL != accel_sens)
    {
        mpu_get_accel_sens(accel_sens);
    }
        
    if(NULL != gyro_sens)
    {
        mpu_get_gyro_sens(gyro_sens);
    }

    if(NULL != mag_sens)
    {
        pp_get_compass_mag_sens_adj(mag_sens);
    }
}

void sensor_test(void)
{
    mpu9250_test();
    ak8963_test();
    bmp280_hal_test();
}

#if 0
void sensor_test(void)
{

    uint8_T buf[19];
    int16_T buf_i16[7] = {0};
    int32_T times = 0;
    uint32_T ms1 = 0;
    uint32_T ms2 = 0;
    uint32_T ms3 = 0;
    uint32_T clk1 = 0;
    uint32_T clk2 = 0;
    uint32_T clk3 = 0;
    uint32_T diffms1 = 0;
    uint32_T diffclk1 = 0;
    uint32_T diffms2 = 0;
    uint32_T diffclk2 = 0;
   
    console_printf("传感器测试开始.\r\n");

    while(1)
    {
        if(g_tx_cplt)
        {
            g_tx_cplt = FALSE;

            /* 启动加计测试 */
            if(0 == times)
            {
                /* 加计数据 */
                get_now(&ms1, &clk1);
                if(HAL_OK != HAL_I2C_Mem_Read_DMA(&g_si_handle, MPU9250_DEV_ADDR,
                            MPU9250_ALL_FIRST_DATA_ADDR, I2C_MEMADD_SIZE_8BIT, buf, MPU9250_ACCEL_DATA_LENGTH))
                {
                    while(1);
                }
                get_now(&ms2, &clk2);
            }

            /* 统计陀螺仪结果 & 启动加计温度陀螺仪测试 */
            if(1 == times)
            {
                get_now(&ms3, &clk3);
                diff_clk(&diffms1, &diffclk1, ms1, clk1, ms2, clk2);
                diff_clk(&diffms2, &diffclk2, ms2, clk2, ms3, clk3);

                buf_i16[0] = ((buf[0]) << 8) | buf[1];
                buf_i16[1] = ((buf[2]) << 8) | buf[3];
                buf_i16[2] = ((buf[4]) << 8) | buf[5];
                console_printf("test%d 加计(%dBytes):\r\n", times, MPU9250_ACCEL_DATA_LENGTH);
                console_printf("DMA读取请求耗时(%dBytes):time: %u ms, %.2fus\r\n", MPU9250_ACCEL_DATA_LENGTH, diffms1,  1.0f * diffclk1 / 84);
                console_printf("数据读取耗时(%dBytes):   time: %u ms, %.2fus\r\n", MPU9250_ACCEL_DATA_LENGTH, diffms2,  1.0f * diffclk2 / 84);
                console_printf("加计  : %7.4f %7.4f %7.4f\r\n",
                        1.0f * buf_i16[0] / accel_sens,
                        1.0f * buf_i16[1] / accel_sens,
                        1.0f * buf_i16[2] / accel_sens);

                /* 加计温度陀螺仪测试 */
                get_now(&ms1, &clk1);
                if(HAL_OK != HAL_I2C_Mem_Read_DMA(&g_si_handle, MPU9250_DEV_ADDR,
                            MPU9250_ALL_FIRST_DATA_ADDR, I2C_MEMADD_SIZE_8BIT, buf, MPU9250_ALL_DATA_LENGTH))
                {
                    while(1);
                }
                get_now(&ms2, &clk2);
            }

            /* 统计加计温度陀螺仪测试 & 启动磁力计测试 */
            if(2 == times)
            {
                get_now(&ms3, &clk3);
                diff_clk(&diffms1, &diffclk1, ms1, clk1, ms2, clk2);
                diff_clk(&diffms2, &diffclk2, ms2, clk2, ms3, clk3);

                buf_i16[0] = ((buf[0]) << 8) | buf[1];
                buf_i16[1] = ((buf[2]) << 8) | buf[3];
                buf_i16[2] = ((buf[4]) << 8) | buf[5];
                buf_i16[3] = ((buf[6]) << 8) | buf[7];
                buf_i16[4] = ((buf[8]) << 8) | buf[9];
                buf_i16[5] = ((buf[10]) << 8) | buf[11];
                buf_i16[6] = ((buf[12]) << 8) | buf[13];
                console_printf("test%d 加计&温度&陀螺仪(total %dBytes):\r\n", times, MPU9250_ALL_DATA_LENGTH);
                console_printf("DMA读取请求耗时(%dBytes):time: %u ms, %.2fus\r\n", MPU9250_ALL_DATA_LENGTH, diffms1,  1.0f * diffclk1 / 84);
                console_printf("数据读取耗时(%dBytes):   time: %u ms, %.2fus\r\n", MPU9250_ALL_DATA_LENGTH, diffms2,  1.0f * diffclk2 / 84);
                console_printf("加计  : %7.4f %7.4f %7.4f\r\n",
                        1.0f * buf_i16[0] / accel_sens,
                        1.0f * buf_i16[1] / accel_sens,
                        1.0f * buf_i16[2] / accel_sens);
                console_printf("陀螺仪: %7.4f %7.4f %7.4f\r\n",
                        1.0f * buf_i16[4] / gyro_sens,
                        1.0f * buf_i16[5] / gyro_sens,
                        1.0f * buf_i16[6] / gyro_sens);

                console_printf("温度  : %7.4f\r\n", (21 + (buf_i16[3] / 321.0f)));

                /* 磁力计测试 */
                /* 磁力计采样频率8Hz(较低)所以延迟 */
                HAL_Delay(100);
                get_now(&ms1, &clk1);
                if(HAL_OK != HAL_I2C_Mem_Read_DMA(&g_si_handle, AK8963_DEV_ADDR,
                            AK8963_REG_FIRST_ADDR, I2C_MEMADD_SIZE_8BIT, buf, AK8963_REG_NUMS))
                {
                    while(1);
                }
                get_now(&ms2, &clk2);
            }

            /* 统计磁力计测试 & 输出数据 */
            if(3 == times)
            {
                get_now(&ms3, &clk3);
                diff_clk(&diffms1, &diffclk1, ms1, clk1, ms2, clk2);
                diff_clk(&diffms2, &diffclk2, ms2, clk2, ms3, clk3);

                console_printf("test%d 磁力计(%dBytes):\r\n", times, AK8963_REG_NUMS);
                console_printf("磁力计DMA读取请求耗时(%dBytes):time: %u ms, %.2fus\r\n", AK8963_REG_NUMS, diffms1,  1.0f * diffclk1 / 84);
                console_printf("磁力计数据读取耗时(%dBytes):   time: %u ms, %.2fus\r\n", AK8963_REG_NUMS, diffms2,  1.0f * diffclk2 / 84);
              
                for(int32_T i = 0; i < AK8963_REG_NUMS; i++)
                {
                    console_printf("0x%02x:0x%02x\r\n", i, buf[i]);
                }

                break;
            }

            times++;
        }
    }

    /* FIXME: 加入BMP280测试 */
    console_printf("传感器测试结束.\r\n");
}
#endif

void sensor_read(void)
{
    ;
}

/* 用于测试 */
#if 0
    uint32_T timestamp1 = HAL_GetTick();
    for(int i = 0; i<9999; i++)
    {
        sensor_read_poll(0xD0, 0x75, &val, 1);
    }
    uint32_T timestamp2 = HAL_GetTick();
    timestamp = timestamp2 - timestamp1;
    console_printf("吞吐率%.02fB/s", 9999.0 * 1000 / timestamp);
#endif

/* 读取FIFO */
#if 0
static int16_T gyro[3];
static int16_T accel[3];
static uint32_T timestamp;
static uint8_T sensor;
static uint8_T more;
static int32_T times = 0;
static int32_T rst = 0;
static int32_T count = 0;
static uint8_T val = 0;

void read_fifo_func(void)
{
    UNUSED(gyro);
    UNUSED(accel);
    UNUSED(timestamp);
    UNUSED(sensor);
    UNUSED(more);
    UNUSED(times);
    UNUSED(rst);
    UNUSED(count);
    UNUSED(val);

    extern bool_T g_mpu_fifo_ready;
    extern int16_T g_int_status;
    int mpu_read_fifo(short *gyro, short *accel, unsigned long *timestamp, unsigned char *sensors, unsigned char *more);

    while(1)
    {

        if(g_mpu_fifo_ready)
        {
            gyro[0] = 0;
            gyro[1] = 0;
            gyro[2] = 0;
            accel[0] = 0;
            accel[1] = 0;
            accel[2] = 0;
            timestamp = 0;
            sensor = 0;
            more = 0;
            count = 0;
            do
            {
                rst = mpu_read_fifo(gyro, accel, &timestamp, &sensor, &more);
                count++;
            }while(more > 0);
            g_mpu_fifo_ready = FALSE;

            if(0 == times % 500)
            {
                console_printf("times:%d, timestamp: %u, sensor: 0x%02x, more: 0x%02x\r\n",
                        times,
                        timestamp,
                        sensor,
                        more);
                console_printf("accel: %7.4f %7.4f %7.4f\r\n",
                        1.0f * accel[0]/accel_sens,
                        1.0f * accel[1]/accel_sens,
                        1.0f * accel[2]/accel_sens);
                console_printf("gyro : %7.4f %7.4f %7.4f\r\n",
                        gyro[0]/gyro_sens,
                        gyro[1]/gyro_sens,
                        gyro[2]/gyro_sens);
                console_printf("\r\n");
            }
            times++;
        }
    }
}
#endif

