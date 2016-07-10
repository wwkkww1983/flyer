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
#include "mpu9250.h"
#include "ak8963.h"
#include "bmp280_hal.h"
#include "sensor.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* board.c使用 */
I2C_HandleTypeDef g_si_handle;

/* 数据解析需要使用 */
static __IO bool_T s_rx_cplt = TRUE;

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
    console_printf("传感器i2c总线初始化完成.\r\n");

    return;
}

/* 初始化时采用轮询模型(避免并发出问题) */
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

/* 工作时采用DMA提升效率 */
void si_read_dma(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n)
{ 
    /* 锁住 */
    s_rx_cplt = FALSE;
    if(HAL_OK != HAL_I2C_Mem_Read_DMA(&g_si_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_T *)buf, (uint16_T)n))
    {
        assert_failed(__FILE__, __LINE__);
    }
    return ;
}

bool_T si_read_ready(void)
{
    return s_rx_cplt;
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
    s_rx_cplt = TRUE;
}

void SENSOR_I2C_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(g_si_handle.hdmarx);
}

/* 测试代码 */
void si_test_poll_rate(int32_T times)
{
    /* 测试吞吐率 */
    uint32_T timestamp1 = 0;
    uint32_T timestamp2 = 0;
    uint32_T timestamp = 0;
    uint8_T val = 0;

    timestamp1 = HAL_GetTick();
    for(int i = 0; i < times; i++)
    {
        si_read_poll(MPU9250_DEV_ADDR, MPU9250_WHO_AM_I_REG_ADDR, &val, 1);
    }
    timestamp2 = HAL_GetTick();
    timestamp = timestamp2 - timestamp1;
    console_printf("轮询吞吐率%.02fB/s\r\n", times * 1000.0f / timestamp);
}

void si_test_dma_rate(void)
{
    uint8_T buf[MPU9250_ALL_DATA_LENGTH]; /* 避免溢出 */
    int16_T buf_i16[7] = {0};
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
    uint16_T accel_sens = 0;
    f32_T gyro_sens = 0;
    int16_T mag_sens[3] = {0};
    f32_T adj[3] = {0.0f};

    /* 获取校正值 */
    sensor_get_sens(&accel_sens, &gyro_sens, mag_sens); 
    
    /* 加计数据 */
    get_now(&ms1, &clk1);
    si_read_dma(MPU9250_DEV_ADDR, MPU9250_ALL_FIRST_DATA_ADDR, buf, MPU9250_ACCEL_DATA_LENGTH); 
    get_now(&ms2, &clk2); 
    while(!si_read_ready()); 
    get_now(&ms3, &clk3); 
    diff_clk(&diffms1, &diffclk1, ms1, clk1, ms2, clk2);
    diff_clk(&diffms2, &diffclk2, ms2, clk2, ms3, clk3); 
    buf_i16[0] = ((buf[0]) << 8) | buf[1];
    buf_i16[1] = ((buf[2]) << 8) | buf[3];
    buf_i16[2] = ((buf[4]) << 8) | buf[5];
    console_printf("加计(%dBytes)DMA读取请求耗时:%ums,%.2fus.\r\n", MPU9250_ACCEL_DATA_LENGTH, diffms1, 1.0f * diffclk1 / 84); 
    console_printf("等待数据耗时:%ums,%.2fus.\r\n", diffms2, 1.0f * diffclk2 / 84);
    console_printf("加计数据:%7.4f %7.4f %7.4f\r\n", 1.0f * buf_i16[0] / accel_sens, 1.0f * buf_i16[1] / accel_sens, 1.0f * buf_i16[2] / accel_sens); 
    
    /* 加计温度陀螺仪测试 */ 
    get_now(&ms1, &clk1);
    si_read_dma(MPU9250_DEV_ADDR, MPU9250_ALL_FIRST_DATA_ADDR, buf, MPU9250_ALL_DATA_LENGTH);
    get_now(&ms2, &clk2);
    while(!si_read_ready()); 
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
    console_printf("加计&温度&陀螺仪(total %dBytes)DMA读取请求耗时:%ums,%.2fus\r\n", MPU9250_ALL_DATA_LENGTH, diffms1,  1.0f * diffclk1 / 84);
    console_printf("加计&温度&陀螺仪等待数据耗时:%ums,%.2fus\r\n", diffms2,  1.0f * diffclk2 / 84); 
    console_printf("加计数据:  %7.4f %7.4f %7.4f\r\n", 1.0f * buf_i16[0] / accel_sens, 1.0f * buf_i16[1] / accel_sens, 1.0f * buf_i16[2] / accel_sens);
    console_printf("陀螺仪数据:%7.4f %7.4f %7.4f\r\n", 1.0f * buf_i16[4] / gyro_sens, 1.0f * buf_i16[5] / gyro_sens, 1.0f * buf_i16[6] / gyro_sens); 
    console_printf("温度数据:  %7.4f\r\n", (21 + (buf_i16[3] / 321.0f)));
   
    /* 磁力计测试 */
    /* 8Hz频率 需要加入延迟(100ms是经验值) */
    HAL_Delay(100);
    get_now(&ms1, &clk1);
    si_read_dma(AK8963_DEV_ADDR, AK8963_DATA_FIRST_ADDR, buf, AK8963_DATA_LENGTH);
    get_now(&ms2, &clk2);
    while(!si_read_ready()); 
    get_now(&ms3, &clk3); 
    diff_clk(&diffms1, &diffclk1, ms1, clk1, ms2, clk2);
    diff_clk(&diffms2, &diffclk2, ms2, clk2, ms3, clk3); 
    buf_i16[0] = buf[2] << 8 | buf[1];
    buf_i16[1] = buf[4] << 8 | buf[3];
    buf_i16[2] = buf[6] << 8 | buf[5];
    adj[0] = (0.5f * (mag_sens[0] - 128) / 128) + 1;
    adj[1] = (0.5f * (mag_sens[1] - 128) / 128) + 1;
    adj[2] = (0.5f * (mag_sens[2] - 128) / 128) + 1;
    console_printf("磁力计(%dBytes)DMA读取请求耗时:%ums,%.2fus\r\n", AK8963_DATA_LENGTH, diffms1, 1.0f * diffclk1 / 84);
    console_printf("磁力计等待数据耗时:%ums,%.2fus\r\n", diffms2, 1.0f * diffclk2 / 84); 
    console_printf("加计数据:%7.4f %7.4f %7.4f\r\n", 1.0f * buf_i16[0] / accel_sens, 1.0f * buf_i16[1] / accel_sens, 1.0f * buf_i16[2] / accel_sens);
    console_printf("磁力计数据:ST1:0x%02x,ST2:0x%02x\r\n", buf[0], buf[7]);
    console_printf("X:%7.4f,Y:%7.4f,Z:%7.4f\r\n", buf_i16[0] * adj[0], buf_i16[1] * adj[1], buf_i16[2] * adj[2]);
}

