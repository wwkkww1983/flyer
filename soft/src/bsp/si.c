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
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* board.c使用 */
I2C_HandleTypeDef g_si_handle;

/* 数据解析需要使用 */
static __IO bool_T s_rx_lock = FALSE;

static void si_tc_lock(void);
static void si_tc_unlock(void);

/********************************** 函数实现区 *********************************/
void si_init(void)
{
    if(HAL_I2C_STATE_RESET != HAL_I2C_GetState(&g_si_handle))
    {
        assert_failed(__FILE__, __LINE__);
    }

    g_si_handle.Instance              = MSI_I2C;
    g_si_handle.Init.ClockSpeed       = MSI_RATE;
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
    while(si_rx_locked()); /* 等待上一次传输完成 */
    si_tc_lock(); /* 锁住资源 */
    if(HAL_OK != HAL_I2C_Mem_Read(&g_si_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    }
    si_tc_unlock();

    return;
}

void si_write_poll(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n)
{
    while(si_rx_locked()); /* 等待上一次传输完成 */
    si_tc_lock(); /* 锁住资源 */
    if(HAL_OK != HAL_I2C_Mem_Write(&g_si_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_T *)buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    }
    si_tc_unlock();

    return;
} 

/* 工作时采用DMA提升效率 */
void si_read_dma(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n)
{
    while(si_rx_locked()); /* 等待上一次传输完成 */
    si_tc_lock(); /* 锁住资源 */

    if(HAL_OK != HAL_I2C_Mem_Read_DMA(&g_si_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_T *)buf, (uint16_T)n))
    {
        assert_failed(__FILE__, __LINE__);
    }
    return ;
}

/* 上次传输完成 Transmit Compelete 锁住 */
inline bool_T si_rx_locked(void)
{
    return s_rx_lock;
}

/* FIXME: lock unlock 原子操作 */
/* 上次传输完成 Transmit Compelete 锁(尚未完成) */
inline static void si_tc_lock(void)
{
    s_rx_lock = TRUE;
}

/* 上次传输完成 Transmit Compelete 解锁(完成) */
inline static void si_tc_unlock(void)
{
    s_rx_lock = FALSE;
}

/* MSI_I2C_EV_IRQHandler & MSI_I2C_ER_IRQHandler 未使用 使用DMA提高效率 */
/* 发生EV ER中断表示出错 */
void MSI_I2C_EV_IRQHandler(void)
{
    ERR_STR("中断失败.");
    /* HAL_I2C_EV_IRQHandler(&g_si_handle); */
}

void MSI_I2C_ER_IRQHandler(void)
{
    ERR_STR("中断失败.");
    /* HAL_I2C_ER_IRQHandler(&g_si_handle); */
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    si_tc_unlock();
}

void MSI_I2C_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(g_si_handle.hdmarx);
}

