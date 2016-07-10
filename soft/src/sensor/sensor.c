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
#include "config.h"
#include "typedef.h"
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
    si_init();
    console_printf("传感器i2c总线 初始化完成.\r\n");
	
    mpu9250_init();
    ak8963_init();
    bmp280_hal_init();

    return;
} 

/* 获取校准参数 */
/* 使用inv_mpu.c */ 
void sensor_get_sens(uint16_T *accel_sens, f32_T *gyro_sens, int16_T *mag_sens)
{

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
#ifdef SENSOR_TEST_DETAIL
    ak8963_sample_period(1);
    si_test_poll_rate(20);
    si_test_dma_rate();
#endif

    mpu9250_test();
    ak8963_test();
    bmp280_hal_test();
}

void sensor_read(void)
{
    ;
}

