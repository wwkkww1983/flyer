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

inline bool_T si_read_ready(void)
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

