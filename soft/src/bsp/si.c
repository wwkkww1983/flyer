/******************************************************************************
 *
 * 文件名  ： sensor i2c接口
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
#include "si.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* board.c使用 */
I2C_HandleTypeDef g_si_handle;

/* 数据解析需要使用 */
uint16_T accel_sens = 0;
f32_T    gyro_sens = 0.0f;
int16_T  mag_sens_adj[3];

bool_T g_tx_cplt = TRUE;

/********************************** 函数实现区 *********************************/
void si_init(void)
{
    if(HAL_I2C_STATE_RESET != HAL_I2C_GetState(&g_si_handle))
    {
        assert_failed(__FILE__, __LINE__);
    }

    g_si_handle.Instance              = SENSOR_I2C;
    g_si_handle.Init.ClockSpeed       = SENSOR_I2C_RATE;
    g_si_handle.Init.DutyCycle        = I2C_DUTYCYCLE_2;
    g_si_handle.Init.OwnAddress1      = 0;
    g_si_handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    g_si_handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    g_si_handle.Init.OwnAddress2      = 0;
    g_si_handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    g_si_handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
   
    if(HAL_OK != HAL_I2C_Init(&g_si_handle))
    {
        assert_failed(__FILE__, __LINE__);
    }
    console_printf("sensor i2c 初始化完成.\r\n");

#if 0
    /* mpu9250初始化 */
    mpu9250_init();


    console_printf("mpu9250 初始化完成.\r\n");


    /* ak8963 初始化 */
    ak8963_init() 
    ak8963_sample_period(10);

    /* FIXME: 完成初始化
    bmp280_init();
    console_printf("bmp280_init 初始化完成.\r\n");
    */

    /* 获取校准参数 */
    mpu_get_accel_sens(&accel_sens);
    mpu_get_gyro_sens(&gyro_sens);
    pp_get_compass_mag_sens_adj(mag_sens_adj);
#endif

    return;
}

/* 初始化时采用轮询模型 */
void si_read_poll(uint8_T dev_addr, uint16_T reg_addr, uint8_T *buf, uint32_T n)
{
    if(HAL_OK != HAL_I2C_Mem_Read(&g_si_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    }

    return;
}

void si_write_poll(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n)
{
    if(HAL_OK != HAL_I2C_Mem_Write(&g_si_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_T *)buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    }

    return;
}

/* SENSOR_I2C_EV_IRQHandler & SENSOR_I2C_ER_IRQHandler 未使用 使用DMA提高效率 */
/* 发生EV ER中断表示出错 */
void SENSOR_I2C_EV_IRQHandler(void)
{
    while(1);
    /* HAL_I2C_EV_IRQHandler(&g_si_handle); */
}

void SENSOR_I2C_ER_IRQHandler(void)
{
    while(1);
    /* HAL_I2C_ER_IRQHandler(&g_si_handle); */
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    g_tx_cplt = TRUE;
}

void SENSOR_I2C_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(g_si_handle.hdmarx);
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

