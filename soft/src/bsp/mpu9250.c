/******************************************************************************
 *
 * 文件名  ： mpu9250.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20150703 
 * 版本号  ： 1.0
 * 文件描述： mpu9250驱动
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
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
#include <math.h>
#include <stm32f4xx_hal.h>
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "ml_math_func.h"
#include "exti.h"
#include "si.h"
#include "mpu9250.h"
#include "lib_math.h"
#include "filter.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
static bool_T s_mpu9250_euler_updated = FALSE; /* 姿态有更新 可以且需要重新平衡 */

/********************************** 函数声明区 *********************************/
static void mpu9250_test(void);
static void run_self_test(void);

/********************************** 函数实现区 *********************************/
/* 初始化 */
void mpu9250_init(void)
{
    uint8_T who_am_i = 0;

    /* 测试i2c是否正常工作 */
    si_read_poll(MPU9250_DEV_ADDR, MPU9250_WHO_AM_I_REG_ADDR, &who_am_i, 1); 
    if(MPU9250_WHO_AM_I_REG_VALUE != who_am_i)
    {
        ERR_STR("MPU9250异常.\r\n");
        return;
    }

    if (mpu_init(NULL) != 0)
    {
        ERR_STR("初始化MPU失败!\r\n");
        return;
    }

    if (mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL)!=0)
    {
        ERR_STR("打开传感器失败.\r\n");
        return;
    }	

    if(mpu_set_sample_rate(MPU9250_SAMPLE_RATE) !=0)
    {
        debug_log("设置主采样率(%d)失败.\r\n", MPU9250_SAMPLE_RATE);
        return;
    }

    if (mpu_set_gyro_fsr(MPU9250_GYRO_FSR)!=0)
    {
        ERR_STR("设置陀螺仪量程失败.\r\n");
        return;
    }

    if (mpu_set_accel_fsr(MPU9250_ACCEL_FSR)!=0)
    {
        ERR_STR("设置加速度计量程失败.\r\n");
        return;
    }

    run_self_test(); 
    mpu9250_test();

    return;
}

/* 更新姿态 */
void mpu9250_update(void)
{
    ;
}

/* 获取姿态(欧拉角) */
void mpu9250_get_euler(f32_T *euler)
{
    ;
}

/* 获取姿态(欧拉角) */
bool_T mpu9250_euler_updated(void)
{
    ;
}

/* 清理上次姿态(欧拉角已经被控制模块使用) */
void mpu9250_clear_euler(void)
{
    ;
}

/* 获取dmp四元数 */
void mpu9250_get_dmp_quat(f32_T *dmp_quat)
{
    ;
}

/* 获取加计数据 */
void mpu9250_get_accel(f32_T *accel)
{
    ;
}

/* 获取陀螺采样最大间隔 */
void mpu9250_get_gyro_interval_max(misc_time_T *interval)
{
    ;
}

/* 获取加计采样最大间隔 */
void mpu9250_get_accel_interval_max(misc_time_T *interval)
{
    ;
}

static void run_self_test(void)
{
    int result = 0;
    long gyro[AXES_NUM] = {0};
    long accel[AXES_NUM] = {0};

    result = mpu_run_6500_self_test(gyro, accel, 0);
    if (result == 0x7)
    {
        /* 自检测试通过 我们需要更新校准数据 与offset寄存器 */
        unsigned char i = 0;
        for(i = 0; i<3; i++)
        {
            gyro[i] = (long)(gyro[i] * 32.8f); /* 运用量程 +-1000dps */
            accel[i] *= 2048.f; /* 运用量程convert to +-16G */
            accel[i] = accel[i] >> 16;
            gyro[i] = (long)(gyro[i] >> 16);
        }

        mpu_set_gyro_bias_reg(gyro);
        mpu_set_accel_bias_6500_reg(accel);
        debug_log("自检通过,设置偏移:\r\n");
        debug_log("accel: %7.4f %7.4f %7.4f\r\n",
                accel[0]/65536.f,
                accel[1]/65536.f,
                accel[2]/65536.f);
        debug_log("gyro : %7.4f %7.4f %7.4f\r\n",
                gyro[0]/65536.f,
                gyro[1]/65536.f,
                gyro[2]/65536.f);
    }
    else
    {
        if (!(result & 0x1))
        {
            debug_log("自检陀螺仪失败.\r\n");
        }
        if (!(result & 0x2))
        {
            debug_log("自检加速度计失败.\r\n");
        }
        if (!(result & 0x4))
        {
            debug_log("自检磁力计失败.\r\n");
        }
    }

    return;
}

static void mpu9250_test(void)
{ 
    ;
} 

