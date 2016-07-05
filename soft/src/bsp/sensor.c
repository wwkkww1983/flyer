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
#include "sensor.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* STM32F4Cube HAL驱动 */
static I2C_HandleTypeDef s_sensor_handle;

/********************************** 函数实现区 *********************************/
void sensor_init(void)
{
    if(HAL_I2C_STATE_RESET != HAL_I2C_GetState(&s_sensor_handle))
    {
        assert_failed(__FILE__, __LINE__);
    }

    s_sensor_handle.Instance              = SENSOR_I2C;
    s_sensor_handle.Init.ClockSpeed       = SENSOR_I2C_RATE;
    s_sensor_handle.Init.DutyCycle        = I2C_DUTYCYCLE_2;
    s_sensor_handle.Init.OwnAddress1      = 0;
    s_sensor_handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    s_sensor_handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    s_sensor_handle.Init.OwnAddress2      = 0;
    s_sensor_handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    s_sensor_handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE; 
    
    if(HAL_OK != HAL_I2C_Init(&s_sensor_handle))
    {
        assert_failed(__FILE__, __LINE__);
    } 
    debug_log("sensor i2c 初始化完成.\r\n");

    /* mpu9250初始化 */
    mpu9250_init();
    console_printf("mpu9250 初始化完成.\r\n");

    /* FIXME: 完成初始化
    bmp280_init();
    console_printf("mpu9250 初始化完成.\r\n");
    */
    
    return;
}

/* 初始化时采用轮询模型 */
void sensor_read_poll(uint8_T dev_addr, uint16_T reg_addr, uint8_T *buf, uint32_T n)
{
    if(HAL_OK != HAL_I2C_Mem_Read(&s_sensor_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    }

    return;
}

void sensor_write_poll(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n)
{ 
    if(HAL_OK != HAL_I2C_Mem_Write(&s_sensor_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_T *)buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    } 

    return;
}

void sensor_test(void)
{
    int32_T i = 0;
    /* 测试BMP280 */
    unsigned char bmp280_addr = 0xED;
    unsigned char val = 0;
    int iMax = 0;
    unsigned char bmp180_reg_addr[] = {
        /* 校验寄存器 */
        0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90,
        0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
        0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1,

        /* 控制寄存器 */
        0xD0, 0xE0, 
        0xF3, 0xF4, 0xF5,
        0xF7, 0xF8, 0xF9,
        0xFA, 0xFB, 0xFC 
    };
    debug_log("BMP280 寄存器值:\r\n");
    iMax = sizeof(bmp180_reg_addr) /sizeof(bmp180_reg_addr[0]);
    for(i=0;i<iMax;i++) 
    { 
        sensor_read_poll(bmp280_addr, bmp180_reg_addr[i], &val, 1);
        debug_log("0x%02x:0x%02x\r\n", bmp180_reg_addr[i], val);
    }
    /* 使用BMP280模式寄存器测试写入 */ 
    unsigned char val1 = 0x00;
    unsigned char val2 = 0x00;
    unsigned char val_w = 0x00;
    sensor_write_poll(bmp280_addr, 0xF4, &val1, 1); /* 0xF4最低两位设置为 00 */
    sensor_read_poll(bmp280_addr, 0xF4, &val1, 1); /* val1 最低两位应该为 00 */
    val_w = val1 | 0x03;
    sensor_write_poll(bmp280_addr, 0xF4, &val_w, 1); /* 0xF4最低两位设置为 11 */
    sensor_read_poll(bmp280_addr, 0xF4, &val2, 1); /* val1 最低两位应该为 03 */
    sensor_read_poll(bmp280_addr, 0xD0, &val, 1); /* id 应该为 0x58 */
    if( (0x00 == (0x03 & val1))
     && (0x03 == (0x03 & val2))
     && (0x58 == val))
    {
        debug_log("BMP280写入测试通过.\r\n");
    }
    else
    {
        debug_log("BMP280写入测试失败.\r\n");
    }
}

