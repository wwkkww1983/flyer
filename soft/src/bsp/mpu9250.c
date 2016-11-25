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
#include "fusion.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
static bool_T s_quat_updated = FALSE; /* 姿态有更新 需要重新平衡 */

static bool_T s_mpu9250_fifo_ready = FALSE; 
static uint8_T s_int_status = 0;

static const signed char s_orientation[9] = MPU9250_ORIENTATION;

static f32_T s_quat[4] = {1.0f, 0.0f, 0.0f, 0.0f}; /* 最终的四元数(初始值必须为:1,0,0,0 表示无旋转) */
static misc_interval_max_T s_quat_interval_max = {0}; /* 四元数采样最大间隔 */

static f32_T s_accel[3] = {0.0f, 0.0f, 1.0f}; /* 最终的加计数据(初始值必须为:0,0,1 表示无旋转) */
static uint16_T s_accel_sens = 0; /* 加计灵敏度 */
static misc_interval_max_T s_accel_interval_max = {0}; /* 加计采样最大间隔 */
static f32_T s_filter_rate = FILTER_ACCEL_RATE; /* 加计滤波比例参数 */

/********************************** 函数声明区 *********************************/
static void run_self_test(void);
static void int_callback(void *argv);
static void mpu9250_set_accel(const f32_T *accel);
static void mpu9250_dmp_update(void);

#if 0
static void tap_callback(unsigned char direction, unsigned char count);
static void android_orient_callback(unsigned char orientation);
#endif

/********************************** 函数实现区 *********************************/
/* 初始化 */
void mpu9250_init(void)
{
    uint8_T who_am_i = 0;
    uint16_T dmp_features = 0;

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

    /* 开启DMP中断 */
    exti_set_callback(int_callback, NULL);

    if (mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL)!=0)
    {
        ERR_STR("打开传感器失败.\r\n");
        return;
    }	

    if(mpu_set_sample_rate(MPU9250_SAMPLE_RATE) !=0)
    {
        debug_log("设置accel+gyro主采样率(%d)失败.\r\n", MPU9250_SAMPLE_RATE);
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

    /* FIXME:为何无效 */
    if (mpu_set_lpf(MPU9250_ACCEL_LPF)!=0)
    {
        ERR_STR("设置加速度计低通滤波器失败.\r\n");
        return;
    }

    run_self_test(); 
    
    /*
     * 初始化 DMP:
     * 1. 注册回调函数
     * 2. 调用dmp_load_motion_driver_firmware().加载inv_mpu_dmp_motion_driver.h的
     *    DMP程序.
     * 3. 加方向矩阵加入DMP.
     * 4. 注册姿态回调函数
     * 5. 调用dmp_enable_feature()使能特性
     * 6. 调用mpu_set_dmp_state(1)启动dmp
     *
     * 不能使用的特性组合:
     * 1. DMP_FEATURE_6X_LP_QUAT < == > DMP_FEATURE_LP_QUAT
     * 2. DMP_FEATURE_SEND_CAL_GYRO < == > DMP_FEATURE_SEND_RAW_GYRO.
     *
     * 已知问题:
     * 如果没有使能DMP_FEATURE_TAP,无论使用dmp_set_fifo_rate设置的中断
     * 频率为多少,dmp生成中断的频率都为200Hz.
     * 为了避免这个问题,需要使能DMP_FEATURE_TAP
     *
     * DMP融合工作于:
     * gyro +-2000dps
     * accel +-2G
     *
     */
    dmp_load_motion_driver_firmware();
    dmp_set_orientation(inv_orientation_matrix_to_scalar(s_orientation));
    //dmp_register_tap_cb(tap_callback);
    //dmp_register_android_orient_cb(android_orient_callback);
    dmp_features = DMP_FEATURE_LP_QUAT
        | DMP_FEATURE_TAP
        //| DMP_FEATURE_ANDROID_ORIENT
        /* 发送原始数据是否会改变 中断频率 */
        | DMP_FEATURE_SEND_RAW_ACCEL
        //| DMP_FEATURE_SEND_RAW_GYRO
        | DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(dmp_features);
    dmp_set_fifo_rate(MPU9250_DMP_SAMPLE_RATE);
    dmp_set_interrupt_mode(DMP_INT_CONTINUOUS);
    /* 该函数会关闭bypass模式 */
    mpu_set_dmp_state(1);

    mpu9250_test();

    mpu_get_accel_sens(&s_accel_sens);

    return;
}

static void mpu9250_test(void)
{ 
    ;
} 

/* 更新姿态 */
void mpu9250_update(void)
{
    mpu9250_dmp_update();
}

static void mpu9250_dmp_update()
{
    int16_T gyro[3] = {0};

    int16_T accel_short[3] = {0};
    f32_T accel_f32[3] = {0};
    f32_T accel_filtered[3] = {0.0f};

    int32_T quat[4] = {0};
    f32_T quat_f32[4] = {0.0f};
    uint32_T sensor_timestamp = 0;
    int16_T sensors = 0;
    uint8_T more = 0; 
    
    if(s_mpu9250_fifo_ready) /* 四元数 就绪 */
    { 
        while(si_rx_locked()); /* 自旋等待i2c空闲 */
        s_mpu9250_fifo_ready = FALSE;
        dmp_read_fifo(gyro, accel_short, (long *)quat, (unsigned long *)&sensor_timestamp, &sensors, &more);
        if (more)
        {
            debug_log("有溢出:%u\r\n", more);
        }

        if (sensors & INV_XYZ_GYRO)
        {
            ERR_STR("异常的陀螺仪输出.");
        }
        if (sensors & INV_XYZ_ACCEL) 
        {
            /* 获取加计采样最大间隔 */
            misc_interval_max_update(&s_accel_interval_max);

            /* mpu9250内部格式转为通用浮点格式 */
            accel_f32[0] = accel_short[0] / (f32_T)s_accel_sens;
            accel_f32[1] = accel_short[1] / (f32_T)s_accel_sens;
            accel_f32[2] = accel_short[2] / (f32_T)s_accel_sens; 

            /* 加计数据滤波 */
            filter_accel(accel_filtered, accel_f32, s_filter_rate);
            mpu9250_set_accel(accel_filtered); /* 发送滤波后的数据 */ 
            
            /* 6轴融合 */
            fusion_accel(); 
        }
        if (sensors & INV_WXYZ_QUAT) /* 陀螺仪3轴融合姿态 */
        { 
            /* 获取quat采样最大间隔 */ 
            misc_interval_max_update(&s_quat_interval_max); 

            /* mpu9250内部格式转为通用浮点格式 */
            quat_f32[0] = (f32_T) quat[0] / ((f32_T)(1L << 30));
            quat_f32[1] = (f32_T) quat[1] / ((f32_T)(1L << 30));
            quat_f32[2] = (f32_T) quat[2] / ((f32_T)(1L << 30));
            quat_f32[3] = (f32_T) quat[3] / ((f32_T)(1L << 30)); 

            /* 更新姿态,便于发送给上位机 */
            mpu9250_set_quat(quat_f32);
        }
    } 
}

/* TODO:设置和获取四元数 加锁 */
void mpu9250_set_quat(const f32_T *quat)
{ 
    s_quat[0] = quat[0];
    s_quat[1] = quat[1];
    s_quat[2] = quat[2];
    s_quat[3] = quat[3]; 
    
    s_quat_updated = TRUE;
} 

/* TODO:设置和获取四元数 加锁 */
void mpu9250_get_quat(f32_T *quat)
{ 
    quat[0] = s_quat[0];
    quat[1] = s_quat[1];
    quat[2] = s_quat[2];
    quat[3] = s_quat[3];
}

void mpu9250_get_quat_interval_max(misc_time_T *interval)
{
    interval->ms = s_quat_interval_max.interval_max.ms;
    interval->clk = s_quat_interval_max.interval_max.clk;
}

void mpu9250_get_accel_interval_max(misc_time_T *interval)
{
    interval->ms = s_accel_interval_max.interval_max.ms;
    interval->clk = s_accel_interval_max.interval_max.clk;
}

/* TODO:设置和获取加计数据 加锁 */
static void mpu9250_set_accel(const f32_T *accel)
{ 
    s_accel[0] = accel[0];
    s_accel[1] = accel[1];
    s_accel[2] = accel[2];
} 

/* TODO:设置和获取加计数据 加锁 */
void mpu9250_get_accel(f32_T *accel)
{ 
    accel[0] = s_accel[0];
    accel[1] = s_accel[1];
    accel[2] = s_accel[2];
}

/* 可以控制浆 */
bool_T mpu9250_updated(void)
{
    return s_quat_updated;
}

/* 姿态已经使用 */
void mpu9250_clear(void)
{
    s_quat_updated = FALSE;
}

static void run_self_test(void)
{
    int result = 0;
    long gyro[3] = {0};
    long accel[3] = {0};

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

static void int_callback(void *argv)
{
    /* 以下代码用于测试中断时间 */
#if 1
    static int32_T times = 0;
    static misc_time_T last_time;
    misc_time_T now;
    misc_time_T diff; 

    /* 计算中断间隔 200Hz 5ms左右 */
    if(0 == times)
    {
        get_now(&last_time);
    }
    else
    {
        get_now(&now);
        diff_clk(&diff, &last_time, &now);

        last_time.ms = now.ms;
        last_time.clk = now.clk;
    }
    times++;
#endif

    /* FIXME: 中断中自旋 性能较差 */
    while(si_rx_locked()); /* 自旋等待i2c空闲 */
    /* 读取中断状态寄存器(用于清中断) */
    si_read_dma(MPU9250_DEV_ADDR, MPU9250_INT_STATUS_REG_ADDR, &s_int_status, 1);

    s_mpu9250_fifo_ready = TRUE;
}

#if 0
/* 关闭回调功能 避免震动影响性能测试 */
static void tap_callback(unsigned char direction, unsigned char count)
{
    switch (direction)
    {
        case TAP_X_UP:
            debug_log("X上.\r\n");
            break;
        case TAP_X_DOWN:
            debug_log("X下.\r\n");
            break;
        case TAP_Y_UP:
            debug_log("Y上.\r\n");
            break;
        case TAP_Y_DOWN:
            debug_log("Y下.\r\n");
            break;
        case TAP_Z_UP:
            debug_log("Z上.\r\n");
            break;
        case TAP_Z_DOWN:
            debug_log("Z下.\r\n");
            break;
        default:
            err_log("tap_callback error.\r\n");
            return;
    }

    debug_log("tap:%d\n", count);
    return;
}

static void android_orient_callback(unsigned char orientation)
{

    switch (orientation)
    {
        case ANDROID_ORIENT_PORTRAIT:
            debug_log("竖屏\r\n");
            break;
        case ANDROID_ORIENT_LANDSCAPE:
            debug_log("横屏\r\n");
            break;
        case ANDROID_ORIENT_REVERSE_PORTRAIT:
            debug_log("反竖屏\r\n");
            break;
        case ANDROID_ORIENT_REVERSE_LANDSCAPE:
            debug_log("反横屏\r\n");
            break;
        default:
            return;
    }
    return;
}
#endif
