/******************************************************************************
 *
 * 文件名  ： sensor.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160112 
 * 版本号  ： 1.0
 * 文件描述： sensor i2c 驱动程序实现
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
#include "board.h"
#include "misc.h"
#include "mpu9250.h"
#include "bmp280.h"
#include "sensor.h"
#include "console.h"

#include "inv_mpu.h"
/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* board.c使用 */
I2C_HandleTypeDef g_sensor_handle;

/* 数据解析需要使用 */
uint16_T accel_sens = 0;
f32_T    gyro_sens = 0.0f;
int16_T  mag_sens_adj[3];

bool_T g_tx_cplt = TRUE;

/********************************** 函数实现区 *********************************/
void sensor_init(void)
{
    if(HAL_I2C_STATE_RESET != HAL_I2C_GetState(&g_sensor_handle))
    {
        assert_failed(__FILE__, __LINE__);
    }

    g_sensor_handle.Instance              = SENSOR_I2C;
    g_sensor_handle.Init.ClockSpeed       = SENSOR_I2C_RATE;
    g_sensor_handle.Init.DutyCycle        = I2C_DUTYCYCLE_2;
    g_sensor_handle.Init.OwnAddress1      = 0;
    g_sensor_handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    g_sensor_handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    g_sensor_handle.Init.OwnAddress2      = 0;
    g_sensor_handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    g_sensor_handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE; 
    
    if(HAL_OK != HAL_I2C_Init(&g_sensor_handle))
    {
        assert_failed(__FILE__, __LINE__);
    } 
    console_printf("sensor i2c 初始化完成.\r\n");

    /* mpu9250初始化 */
    mpu9250_init();
    mpu_get_accel_sens(&accel_sens);
    mpu_get_gyro_sens(&gyro_sens);
    pp_get_compass_mag_sens_adj(mag_sens_adj);

    console_printf("mpu9250 初始化完成.\r\n");

    /* FIXME: 完成初始化
    bmp280_init();
    console_printf("bmp280_init 初始化完成.\r\n");
    */

    return;
}

/* 初始化时采用轮询模型 */
void sensor_read_poll(uint8_T dev_addr, uint16_T reg_addr, uint8_T *buf, uint32_T n)
{
    if(HAL_OK != HAL_I2C_Mem_Read(&g_sensor_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    }

    return;
}

void sensor_write_poll(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n)
{ 
    if(HAL_OK != HAL_I2C_Mem_Write(&g_sensor_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_T *)buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    } 

    return;
}


uint8_T buf[22];
int32_T times = 0;
uint32_T ms1 = 0;
uint32_T ms2 = 0;
uint32_T ms3 = 0;
uint32_T clk1 = 0;
uint32_T clk2 = 0;
uint32_T clk3 = 0;
uint32_T diffms = 0;
uint32_T diffclk = 0;
void sensor_test(void)
{
    /* 获取校准参数 */
    mpu_get_accel_sens(&accel_sens);
    mpu_get_gyro_sens(&gyro_sens);
    pp_get_compass_mag_sens_adj(mag_sens_adj);

    while(1)
    {
        if(g_tx_cplt)
        {
            g_tx_cplt = FALSE; 

#if 0
            if(1 == times)
            {
                get_now(&ms3, &clk3); 
                diff_clk(&diffms, &diffclk, ms1, clk1, ms3, clk3); 
                console_printf("6Bytes 包发送时间:time: %u ms, %.2fus", diffms,  1.0f * diffclk / 84)
            }
            /* 加计数据 */
            get_now(&ms1, &clk1);
            if(HAL_OK != HAL_I2C_Mem_Read_DMA(&g_sensor_handle, MPU9250_DEV_ADDR,
                        MPU9250_ACCEL_DATA_ADDR, I2C_MEMADD_SIZE_8BIT, buf, 6))
            {
                while(1);
            }
            get_now(&ms2, &clk2); 
            diff_clk(&diffms, &diffclk, ms1, clk1, ms2, clk2);

#else
            if(1 == times)
            {
                get_now(&ms3, &clk3); 
                diff_clk(&diffms, &diffclk, ms1, clk1, ms3, clk3); 
                console_printf("14Bytes 包发送时间:time: %u ms, %.2fus", diffms,  1.0f * diffclk / 84);
            }
            /* 加计&陀螺 数据 */
            get_now(&ms1, &clk1);
            if(HAL_OK != HAL_I2C_Mem_Read_DMA(&g_sensor_handle, MPU9250_DEV_ADDR,
                        MPU9250_ACCEL_DATA_ADDR, I2C_MEMADD_SIZE_8BIT, buf, 14))
            {
                while(1);
            }
            get_now(&ms2, &clk2); 
            diff_clk(&diffms, &diffclk, ms1, clk1, ms2, clk2); 
            console_printf("请求发送时间:time: %u ms, %.2fus", diffms,  1.0f * diffclk / 84);

#if 0
            /* 加计&陀螺&磁力计 数据 */
            get_now(&ms1, &clk1);
            if(HAL_OK != HAL_I2C_Mem_Read_DMA(&g_sensor_handle, MPU9250_DEV_ADDR,
                        MPU9250_ACCEL_DATA_ADDR, I2C_MEMADD_SIZE_8BIT, buf, 22))
            {
                while(1);
            }
            get_now(&ms2, &clk2); 
            diff_clk(&diffms, &diffclk, ms1, clk1, ms2, clk2);
#endif

            if(0 == times % 500)
            { 
                int16_T buf_i16[10] = {0};

                buf_i16[3] = ((buf[6]) << 8) | buf[7];

                buf_i16[0] = ((buf[0]) << 8) | buf[1];
                buf_i16[1] = ((buf[2]) << 8) | buf[3];
                buf_i16[2] = ((buf[4]) << 8) | buf[5];

                buf_i16[4] = ((buf[8]) << 8) | buf[9];
                buf_i16[5] = ((buf[10]) << 8) | buf[11];
                buf_i16[6] = ((buf[12]) << 8) | buf[13];


                console_printf("time: %u ms, %.2fus, temp: %ld", 
                        ms2,  1.0f * clk2 / 84,
                        (long)((35 + (1.0f * buf_i16[3] / 321) * 65536L)));

                console_printf("accel: %7.4f %7.4f %7.4f\r\n",
                        1.0f * buf_i16[0] / accel_sens,
                        1.0f * buf_i16[1] / accel_sens,
                        1.0f * buf_i16[2] / accel_sens);

                console_printf("gyro:  %7.4f %7.4f %7.4f\r\n",
                        1.0f * buf_i16[4] / gyro_sens,
                        1.0f * buf_i16[5] / gyro_sens,
                        1.0f * buf_i16[6] / gyro_sens); 

                /* 磁力计读取 原理上有问题 */
                buf_i16[7] = ((buf[15]) << 8) | buf[16];
                buf_i16[8] = ((buf[17]) << 8) | buf[18];
                buf_i16[9] = ((buf[19]) << 8) | buf[20];
                console_printf("compass ");
                if (!(buf[14] & MPU9250_AKM_DATA_READY)
                  || (buf[14] & MPU9250_AKM_DATA_OVERRUN)
                  || (buf[21] & MPU9250_AKM_OVERFLOW))
                {
                    console_printf("data error: 0x%04x 0x%03x 0x%04x, 0x%02x, 0x%02x.\r\n\r\n",
                            buf_i16[7] * mag_sens_adj[0] >> 8,
                            buf_i16[8] * mag_sens_adj[1] >> 8,
                            buf_i16[9] * mag_sens_adj[2] >> 8,
                            buf[14], buf[21]);
                }
                else 
                {
                    console_printf(": 0x%04x 0x%03x 0x%04x, 0x%02x, 0x%02x.\r\n\r\n",
                            buf_i16[7] * mag_sens_adj[0] >> 8,
                            buf_i16[8] * mag_sens_adj[1] >> 8,
                            buf_i16[9] * mag_sens_adj[2] >> 8,
                            buf[14], buf[21]);
                }
            }
#endif

            times++;
        }
    }
}

void sensor_read(void)
{
    ;
}

/* SENSOR_I2C_EV_IRQHandler & SENSOR_I2C_ER_IRQHandler 未使用 使用DMA提高效率 */
/* 发生EV ER中断表示出错 */
void SENSOR_I2C_EV_IRQHandler(void)
{
    while(1);
    /* HAL_I2C_EV_IRQHandler(&g_sensor_handle); */
}

void SENSOR_I2C_ER_IRQHandler(void)
{
    while(1);
    /* HAL_I2C_ER_IRQHandler(&g_sensor_handle); */
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    g_tx_cplt = TRUE;
}
	
void SENSOR_I2C_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(g_sensor_handle.hdmarx);
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

