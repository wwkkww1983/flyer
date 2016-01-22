/******************************************************************************
 *
 * 文件名  ： data.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160116 
 * 版本号  ： v1.0
 * 文件描述： accel gyro compass数据处理
 * 版权说明： Copyright (c) 2000-2020   烽火通信科技股份有限公司
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/


/*---------------------------------- 预处理区 ---------------------------------*/
/* 消除中文打印警告 */
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include "data.h"
#include "imu.h"
#include "inv_mpu.h"
#include "mpu9250.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"


/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* 动态检测的参数 */
static uint16_T g_accel_sens = 0;
static f32_T    g_gyro_sens = 0.0f;
static int16_T g_mag_sens_adj[3];

/********************************** 函数声明区 *********************************/

/********************************** 变量实现区 *********************************/


/********************************** 函数实现区 *********************************/
void parse_accel(accel_T *accel, const data_T *data)
{
    int16_T buf_i16[3] = {0};

    assert_param(accel_E == data->type);

    buf_i16[0] = ((data->buf[0]) << 8) | data->buf[1];
    buf_i16[1] = ((data->buf[2]) << 8) | data->buf[3];
    buf_i16[2] = ((data->buf[4]) << 8) | data->buf[5];

    accel->data[0] = 1.0 * buf_i16[0] / g_accel_sens;
    accel->data[1] = 1.0 * buf_i16[1] / g_accel_sens;
    accel->data[2] = 1.0 * buf_i16[2] / g_accel_sens;

    accel->time = data->time;
}

void parse_gyro(gyro_T *gyro, const data_T *data)
{
    int16_T buf_i16[3] = {0};

    assert_param(gyro_E == data->type);

    buf_i16[0] = ((data->buf[0]) << 8) | data->buf[1];
    buf_i16[1] = ((data->buf[2]) << 8) | data->buf[3];
    buf_i16[2] = ((data->buf[4]) << 8) | data->buf[5];

    gyro->data[0] = 1.0 * buf_i16[0] / g_gyro_sens;
    gyro->data[1] = 1.0 * buf_i16[1] / g_gyro_sens;
    gyro->data[2] = 1.0 * buf_i16[2] / g_gyro_sens;

    gyro->time = data->time;
}

void parse_compass(compass_T *compass, const data_T *data)
{
    int16_T buf_i16[3] = {0};

    assert_param(compass_E == data->type);

    /* compass 参数检测 */
#define AKM_DATA_READY      (0x01)
#define AKM_DATA_OVERRUN    (0x02)
#define AKM_OVERFLOW        (0x80)
    if ((!(data->buf[0] & AKM_DATA_READY))
    || (data->buf[0] & AKM_DATA_OVERRUN)
    || (data->buf[7] & AKM_OVERFLOW))
    {
        ERR_STR("compass数据错误.");
        while(1);
    }
#undef AKM_DATA_READY
#undef AKM_DATA_OVERRUN
#undef AKM_OVERFLOW

    buf_i16[0] = ((data->buf[2]) << 8) | data->buf[1];
    buf_i16[1] = ((data->buf[4]) << 8) | data->buf[3];
    buf_i16[2] = ((data->buf[6]) << 8) | data->buf[5];

    compass->data[0] = 1.0 * (((long)buf_i16[0] * g_mag_sens_adj[0]) >> 8);
    compass->data[1] = 1.0 * (((long)buf_i16[1] * g_mag_sens_adj[1]) >> 8);
    compass->data[2] = 1.0 * (((long)buf_i16[2] * g_mag_sens_adj[2]) >> 8);

    compass->time = data->time;
}

void data_config(void)
{ 
    mpu_get_accel_sens(&g_accel_sens);
    mpu_get_gyro_sens(&g_gyro_sens);

    pp_get_compass_mag_sens_adj(g_mag_sens_adj);

    debug_log("MPU9250校正参数获取完毕.");
}

