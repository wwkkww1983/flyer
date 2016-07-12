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
#include <stm32f4xx_hal.h>
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "ml_math_func.h"
#include "exti.h"
#include "si.h"
#include "mpu9250.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
#if 0
__IO bool_T g_pp_fifo_ready = FALSE; 
static const signed char s_orientation[9] = MPU9250_ORIENTATION;
#endif

/* 灵敏度值 */
static uint16_T s_accel_sens = 0;
static f32_T s_gyro_sens = 0;

/********************************** 函数声明区 *********************************/
static void run_self_test(void);

#if 0
static void int_callback(void *argv);
static void tap_callback(unsigned char direction, unsigned char count);
static void android_orient_callback(unsigned char orientation);
#endif

/********************************** 函数实现区 *********************************/
/* 初始化 */
void mpu9250_init(void)
{
    uint8_T who_am_i = 0;
    //uint16_T dmp_features = 0;

    /* 测试i2c是否正常工作 */
    si_read_poll(MPU9250_DEV_ADDR, MPU9250_WHO_AM_I_REG_ADDR, &who_am_i, 1); 
    if(MPU9250_WHO_AM_I_REG_VALUE != who_am_i)
    {
        console_printf("MPU9250异常.\r\n");
        while(1);
    }

    if (mpu_init(NULL) != 0)
    {
        console_printf("初始化MPU失败!\r\n");
        return;
    }

    if (mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL|INV_XYZ_COMPASS)!=0)
    {
        console_printf("打开传感器失败.\r\n");
        return;
    }	

    if(mpu_set_sample_rate(MPU9250_SAMPLE_RATE) !=0)
    {
        console_printf("设置accel+gyro主采样率(%d)失败.\r\n", MPU9250_SAMPLE_RATE);
        return;
    }

    if (mpu_set_gyro_fsr(MPU9250_GYRO_FSR)!=0)
    {
        console_printf("设置陀螺仪量程失败.\r\n");
        return;
    }

    if (mpu_set_accel_fsr(MPU9250_ACCEL_FSR)!=0)
    {
        console_printf("设置加速度计量程失败.\r\n");
        return;
    }

    run_self_test(); 
    
#if 0
    /* 开启DMP中断 */
    exti_set_callback(int_callback, NULL);
    if (mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL)!=0)
    {
        console_printf("设置MPU FIFO失败.\r\n");
        return;
    } 
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
    dmp_features = DMP_FEATURE_6X_LP_QUAT
        | DMP_FEATURE_TAP
        | DMP_FEATURE_ANDROID_ORIENT
        | DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(dmp_features);
    dmp_set_fifo_rate(MPU9250_DMP_SAMPLE_RATE);
    dmp_set_interrupt_mode(DMP_INT_CONTINUOUS);
    /* 该函数会关闭bypass模式 */
    mpu_set_dmp_state(1);
    mpu_set_bypass(1); /* 打开bypass */
#endif 
    
    /* 初始化灵敏度值 */
    mpu_get_accel_sens(&s_accel_sens); 
    mpu_get_gyro_sens(&s_gyro_sens); 

    return;
}

void mpu9250_test(void)
{
    uint8_T buf[MPU9250_ATG_LENGTH]; /* 避免溢出 */
    uint32_T type = 0;
    misc_time_T time1, time2, time3;
    misc_time_T diff1, diff2;
    mpu9250_val_T mpu9250;

    /* 加计温度陀螺仪测试 */ 
    get_now(&time1); 
    type = ACCEL_TYPE | GYRO_TYPE;
    mpu9250_read(type, buf);
    get_now(&time2);
    while(!si_read_ready()); 
    get_now(&time3);
    diff_clk(&diff1, &time1, &time2);
    diff_clk(&diff2, &time2, &time3); 
    mpu9250_parse(&mpu9250, buf, type);
    console_printf("加计&温度&陀螺仪(total %dBytes)DMA读取请求耗时:%ums,%.2fus\r\n",
            MPU9250_ATG_LENGTH, diff1.ms,  1.0f * diff1.clk/ 84);
    console_printf("加计&温度&陀螺仪等待数据耗时:%ums,%.2fus\r\n",
            diff2.ms,  1.0f * diff2.clk / 84); 
    console_printf("加计数据:  %7.4f %7.4f %7.4f\r\n", mpu9250.accel[0], mpu9250.accel[1], mpu9250.accel[2]);
    console_printf("陀螺仪数据:%7.4f %7.4f %7.4f\r\n", mpu9250.gyro[0], mpu9250.gyro[1], mpu9250.gyro[2]);
    console_printf("温度数据:  %7.4f\r\n", (21 + (buf[6] << 8 | buf[7]) / 321.0f));
} 

/* TODO: 实现多种数据读取 */
void mpu9250_read(uint32_T type, const uint8_T *buf)
{
    if((ACCEL_TYPE & type) /* 加计&陀螺仪 为了效率和简化一口气读取(包括温度) */
    && (GYRO_TYPE & type))
    { 
        si_read_dma(MPU9250_DEV_ADDR, MPU9250_ATG_REG_ADDR, buf, MPU9250_ATG_LENGTH);
        return;
    } 
    
    /* 为了便于磁力计融合 需要加入加计数据 */
    if((ACCEL_TYPE & type)
    && (QUAT_TYPE & type))
    {
        console_printf("四元数获取未实现.\r\n",);
        return;
    }
    
    err_log("无效类型数据:0x%08x.\r\n", type);
    while(1);
}

/*
 * 返回 0 成功
 * 其他   无效数据
 *
 * */
int32_T mpu9250_parse(mpu9250_val_T *mpu9250, const uint8_T *buf, uint32_T type)
{
    f32_T accel[3];
    f32_T gyro[3];
    f32_T quat[4];

    int16_T buf_i16[7] = {0}; /* accel 3Bytes, temp 2Bytes, gyro 6Bytes */

    if((ACCEL_TYPE & type) /* 加计&陀螺仪 为了效率和简化一口气读取(包括温度) */
    && (GYRO_TYPE & type))
    { 
        /* accel */
        buf_i16[0] = ((buf[0]) << 8) | buf[1];
        buf_i16[1] = ((buf[2]) << 8) | buf[3];
        buf_i16[2] = ((buf[4]) << 8) | buf[5];
        mpu9250->accel[0] = buf_i16[0] / s_accel_sens;
        mpu9250->accel[1] = buf_i16[1] / s_accel_sens;
        mpu9250->accel[2] = buf_i16[2] / s_accel_sens;

        /* gyro */
        buf_i16[4] = ((buf[8]) << 8) | buf[9];
        buf_i16[5] = ((buf[10]) << 8) | buf[11];
        buf_i16[6] = ((buf[12]) << 8) | buf[13]; 
        mpu9250->gyro[0] = buf_i16[4] / s_accel_sens;
        mpu9250->gyro[1] = buf_i16[5] / s_accel_sens;
        mpu9250->gyro[2] = buf_i16[6] / s_accel_sens;

        mpu9250->type = type;
        /* 温度未使用
           buf_i16[3] = ((buf[6]) << 8) | buf[7];
           f32_T temp = 21 + (buf_i16[3] / 321.0f)));
           */ 

        UNUSED(quat);
        return 0;
    } 
    
    /* 为了便于磁力计融合 需要加入加计数据 */
    if((ACCEL_TYPE & type)
    && (QUAT_TYPE & type))
    {
        console_printf("四元数解析未实现.\r\n",);

        UNUSED(accel);
        UNUSED(gyro);
        return -1;
    }
    
    err_log("无效类型数据:0x%08x.\r\n", type);
    return -1;
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
#if 0
        console_printf("自检通过,设置偏移:\r\n");
        console_printf("accel: %7.4f %7.4f %7.4f\r\n",
                accel[0]/65536.f,
                accel[1]/65536.f,
                accel[2]/65536.f);
        console_printf("gyro : %7.4f %7.4f %7.4f\r\n",
                gyro[0]/65536.f,
                gyro[1]/65536.f,
                gyro[2]/65536.f);
#endif
    }
    else
    {
        if (!(result & 0x1))
        {
            console_printf("自检陀螺仪失败.\r\n");
        }
        if (!(result & 0x2))
        {
            console_printf("自检加速度计失败.\r\n");
        }
        if (!(result & 0x4))
        {
            console_printf("自检磁力计失败.\r\n");
        }
    }

    return;
}

#if 0
/* 读取FIFO */
static int16_T gyro[3];
static int16_T accel[3];
static uint32_T timestamp;
static uint8_T sensor;
static uint8_T more;
static int32_T times = 0;
static int32_T rst = 0;
static int32_T count = 0;
static uint8_T val = 0;
__IO int16_T g_int_status = 0;
static void read_fifo_func(void)
{
    UNUSED(gyro);
    UNUSED(accel);
    UNUSED(timestamp);
    UNUSED(sensor);
    UNUSED(more);
    UNUSED(times);
    UNUSED(rst);
    UNUSED(count);
    UNUSED(val);

    extern bool_T g_pp_fifo_ready;
    extern int16_T g_int_status;
    int mpu_read_fifo(short *gyro, short *accel, unsigned long *timestamp, unsigned char *sensors, unsigned char *more);

    while(1)
    {

        if(g_pp_fifo_ready)
        {
            gyro[0] = 0;
            gyro[1] = 0;
            gyro[2] = 0;
            accel[0] = 0;
            accel[1] = 0;
            accel[2] = 0;
            timestamp = 0;
            sensor = 0;
            more = 0;
            count = 0;
            do
            {
                rst = mpu_read_fifo(gyro, accel, &timestamp, &sensor, &more);
                count++;
            }while(more > 0);
            g_pp_fifo_ready = FALSE;

            if(0 == times % 500)
            {
                console_printf("times:%d, timestamp: %u, sensor: 0x%02x, more: 0x%02x\r\n",
                        times,
                        timestamp,
                        sensor,
                        more);
                console_printf("accel: %7.4f %7.4f %7.4f\r\n",
                        1.0f * accel[0]/accel_sens,
                        1.0f * accel[1]/accel_sens,
                        1.0f * accel[2]/accel_sens);
                console_printf("gyro : %7.4f %7.4f %7.4f\r\n",
                        gyro[0]/gyro_sens,
                        gyro[1]/gyro_sens,
                        gyro[2]/gyro_sens);
                console_printf("\r\n");
            }
            times++;
        }
    }
}
#endif

#if 0
static int32_T times = 0;
static misc_time_T last_time;
static misc_time_T now;
static misc_time_T diff;
static void int_callback(void *argv)
{
    /* 计算中断间隔 200Hz 5ms左右 */
    if(1 == times)
    {
        get_now(&last_time);
    }
    else if(2 == times)
    {
        get_now(&now);
        diff_clk(&diff, &last_time, &now);
    }
    times++;

    g_pp_fifo_ready = TRUE;
}

/* 关闭回调功能 避免震动影响性能测试 */
static void tap_callback(unsigned char direction, unsigned char count)
{
    switch (direction)
    {
        case TAP_X_UP:
            console_printf("X上.\r\n");
            break;
        case TAP_X_DOWN:
            console_printf("X下.\r\n");
            break;
        case TAP_Y_UP:
            console_printf("Y上.\r\n");
            break;
        case TAP_Y_DOWN:
            console_printf("Y下.\r\n");
            break;
        case TAP_Z_UP:
            console_printf("Z上.\r\n");
            break;
        case TAP_Z_DOWN:
            console_printf("Z下.\r\n");
            break;
        default:
            err_log("tap_callback error.\r\n");
            return;
    }

    console_printf("tap:%d\n", count);
    return;
}

static void android_orient_callback(unsigned char orientation)
{

    switch (orientation)
    {
        case ANDROID_ORIENT_PORTRAIT:
            console_printf("竖屏肖像\n");
            break;
        case ANDROID_ORIENT_LANDSCAPE:
            console_printf("横屏风景\n");
            break;
        case ANDROID_ORIENT_REVERSE_PORTRAIT:
            console_printf("反竖屏肖像\n");
            break;
        case ANDROID_ORIENT_REVERSE_LANDSCAPE:
            console_printf("反横屏风景\n");
            break;
        default:
            return;
    }
    return;
}

#endif

