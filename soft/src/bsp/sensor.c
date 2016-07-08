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
#include "mpu9250.h"
#include "bmp280.h"
#include "sensor.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* board.c使用 */
I2C_HandleTypeDef g_sensor_handle;

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

void sensor_test(void)
{
    ;
}

void SENSOR_I2C_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&g_sensor_handle);
}

void SENSOR_I2C_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&g_sensor_handle);
}

void SENSOR_I2C_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(g_sensor_handle.hdmarx);
}

