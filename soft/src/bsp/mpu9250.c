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
static bool_T s_mpu9250_euler_updated = FALSE;      /* 姿态有更新 可以且需要重新平衡 */
MPU9250_ACCEL_T s_accel = {0};                      /* 最新加计数据 */
MPU9250_GYRO_T  s_gyro = {0};                       /* 最新陀螺数据 */
f32_T s_euler[EULER_MAX] = {0};                     /* 最新姿态,mpu9250模块使用欧拉角,融合算法使用四元数 */
misc_interval_max_T s_mpu9250_interval_max = {0};   /* 采样间隔 */

/********************************** 函数声明区 *********************************/
static void mpu9250_test(void);
static void run_self_test(void);

/********************************** 函数实现区 *********************************/
/* 初始化 */
void mpu9250_init(void)
{
    uint8_T who_am_i = 0;
    int32_T i = 0;

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

    /* 内部数据结构初始化(初始化为无旋转) */
    s_mpu9250_euler_updated = FALSE;
    for(i = 0; i< EULER_MAX; i++)
    {
        s_euler[i] = 0.0f;
    } 
    s_accel.data[AXES_X] = 0.0f;
    s_accel.data[AXES_Y] = 0.0f;
    s_accel.data[AXES_Z] = 1.0f;
    if(0 != mpu_get_accel_sens(&s_accel.sens))
    {
        ERR_STR("获取加计灵敏度失败.\r\n");
        return;
    }
    s_gyro.data[EULER_THETA] = 0.0f;
    s_gyro.data[EULER_PHI] = 0.0f;
    s_gyro.data[EULER_PSI] = 0.0f;
    if(0 != mpu_get_gyro_sens(&s_gyro.sens))
    {
        ERR_STR("获取陀螺灵敏度失败.\r\n");
        return;
    }

    return;
}

/* 更新姿态 */
void mpu9250_update(void)
{
    static bool_T first_run = TRUE;
    static uint8_T s_buf[MPU9250_ATG_LENGTH] = {0};     /* dma读取缓冲 */
    uint8_T buf[MPU9250_ATG_LENGTH] = {0};              /* dma读取缓冲的备份 用于提高并行度 */
    int16_T buf_i16[AXES_NUM] = {0};                    /* 数据处理的临时变量 */
    int32_T i = 0;

    /* 首次 启动采样 */
    if(first_run)
    {
        si_read_dma(MPU9250_DEV_ADDR, MPU9250_ATG_REG_ADDR, s_buf, MPU9250_ATG_LENGTH);
        first_run = FALSE;
        return;
    }

    /* 其他 保存数据 后 启动采样 */
    if(!si_rx_locked()) /* 上一帧数据已达 */
    { 
        /* step1: 备份数据用于尽快启动采样提高并行度 */
        for(i = 0; i < MPU9250_ATG_LENGTH; i++)
        {
            buf[i] = s_buf[i];
        }

        /* step2: 开始采样 */
        si_read_dma(MPU9250_DEV_ADDR, MPU9250_ATG_REG_ADDR, s_buf, MPU9250_ATG_LENGTH);
        misc_interval_max_update(&s_mpu9250_interval_max);

        /* step3: 数据处理 */
        /* 加计 */
        buf_i16[AXES_X] = ((buf[0]) << 8) | buf[1];
        buf_i16[AXES_Y] = ((buf[2]) << 8) | buf[3];
        buf_i16[AXES_Z] = ((buf[4]) << 8) | buf[5];
        s_accel.data[AXES_X] = buf_i16[AXES_X] / (f32_T)(s_accel.sens);
        s_accel.data[AXES_Y] = buf_i16[AXES_Y] / (f32_T)(s_accel.sens);
        s_accel.data[AXES_Z] = buf_i16[AXES_Z] / (f32_T)(s_accel.sens);
        /* 陀螺仪 */
        buf_i16[EULER_THETA] = ((buf[8])  << 8) | buf[9];
        buf_i16[EULER_PHI]   = ((buf[10]) << 8) | buf[11];
        buf_i16[EULER_PSI]   = ((buf[12]) << 8) | buf[13];
        s_gyro.data[EULER_THETA] = buf_i16[EULER_THETA] / (f32_T)(s_gyro.sens);
        s_gyro.data[EULER_PHI] = buf_i16[EULER_PHI] / (f32_T)(s_gyro.sens);
        s_gyro.data[EULER_PSI] = buf_i16[EULER_PSI] / (f32_T)(s_gyro.sens);

        /* step4: 滤波 + 融合 */
        /* filter_fusion(s_euler, s_gyro, s_accel); */

        s_mpu9250_euler_updated = TRUE;
    }
}

/* 获取姿态(欧拉角) */
void mpu9250_get_euler(f32_T *euler)
{
    euler[EULER_THETA] = s_euler[EULER_THETA];
    euler[EULER_PHI] = s_euler[EULER_PHI];
    euler[EULER_PSI] = s_euler[EULER_PSI];
}

/* 获取姿态(欧拉角) */
bool_T mpu9250_euler_updated(void)
{
    return s_mpu9250_euler_updated;
}

/* 清理上次姿态(欧拉角已经被控制模块使用) */
void mpu9250_clear_euler(void)
{
    s_mpu9250_euler_updated = FALSE;
}

/* 获取dmp四元数 mpu9250硬dmp融合算法不便于加计滤波 不使用 */
void mpu9250_get_dmp_quat(f32_T *dmp_quat)
{
    ERR_STR("mpu9250_get_dmp_quat 未实现.\r\n")
}

/* 获取加计数据 */
void mpu9250_get_accel(f32_T *accel)
{
    accel[AXES_X] = s_accel.data[AXES_X];
    accel[AXES_Y] = s_accel.data[AXES_Y];
    accel[AXES_Z] = s_accel.data[AXES_Z];
}

/* 获取陀螺采样最大间隔 */
void mpu9250_get_interval_max(misc_time_T *interval)
{
    interval->ms = s_mpu9250_interval_max.interval_max.ms;
    interval->clk = s_mpu9250_interval_max.interval_max.clk;
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

