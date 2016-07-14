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
#include "si.h"
#include "mpu9250.h"
#include "ak8963.h"
#include "bmp280_hal.h"
#include "console.h"
#include "sensor.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* 传感器回传的解析前数据 */ 
static uint8_T s_buf[MPU9250_ATG_LENGTH]; /* 取较大值 避免溢出 */ 
static uint32_T s_buf_type; /* 用于存储上次数据类型 */
uint32_t s_tick = 0; /* 用于存储采集时间 */

/* 传感器回传的解析后数据 */
sensor_data_T s_data;
static bool_T s_data_ready = FALSE;

static void parse(void);

/********************************** 函数实现区 *********************************/
void sensor_init(void)
{
    si_init();
    debug_log("传感器i2c总线 初始化完成.\r\n");

    mpu9250_init();
    ak8963_init();
    bmp280_hal_init();

    return;
} 

void sensor_test(void)
{ 
    mpu9250_test();

    /* AK8963 8Hz频率 需要加入延迟(100ms是经验值) */
    HAL_Delay(100);
    ak8963_test();
    bmp280_hal_test();
} 

void sensor_read(void)
{
    /* 硬解算不使用buf */
    s_buf_type = QUAT_TYPE;
    mpu9250_read(s_buf_type, NULL);
    parse();
} 

static void parse(void)
{ 
    s_data.type = s_buf_type;
    s_data.time = s_tick;

    if(QUAT_TYPE & s_buf_type)
    {
        /* 硬解算不使用buf */
        if(0 != mpu9250_parse(&(s_data.mpu9250), NULL, QUAT_TYPE))
        { 
            s_data_ready = FALSE;
            return;
        }
    }
    else if((ACCEL_TYPE & s_buf_type)
    && (GYRO_TYPE & s_buf_type))
    { 
        if(0 != mpu9250_parse(&(s_data.mpu9250), s_buf, ACCEL_TYPE | GYRO_TYPE))
        { 
            s_data_ready = FALSE;
            return;
        }
    } 
    else if(COMPASS_TYPE & s_buf_type)
    {
        if(0 != ak8963_parse(&(s_data.ak8963), s_buf))
        { 
            s_data_ready = FALSE;
            return;
        }
    }
    else
    { 
        err_log("无效类型数据:0x%08x.\r\n", s_buf_type); 
        while(1);
    }

    s_data_ready = TRUE;
    return;
}

void sensor_get_data(sensor_data_T **data)
{
    *data = &s_data;
}

inline bool_T sensor_data_ready(void)
{
    return s_data_ready;
}

