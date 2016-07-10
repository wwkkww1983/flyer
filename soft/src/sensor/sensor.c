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
#include "systick.h"
#include "inv_mpu.h"
#include "mpu9250.h"
#include "ak8963.h"
#include "bmp280_hal.h"
#include "console.h"
#include "sensor.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
uint16_T s_accel_sens = 0;
f32_T s_gyro_sens = 0;
static f32_T s_ak8963_adj[3] = {0};

/* 传感器回传的解析前数据 */ 
static uint8_T s_buf[MPU9250_DATA_LENGTH]; /* 取较大值 避免溢出 */ 
static sensor_data_type_T s_buf_type;
uint32_t s_tick = 0;

/* 传感器回传的解析后数据 */
sensor_data_T s_data;
static bool_T s_data_ready = FALSE;

static void parse(void);

/********************************** 函数实现区 *********************************/
void sensor_init(void)
{
    int16_T mag_sens[3] = {0};

    si_init();
    console_printf("传感器i2c总线 初始化完成.\r\n");

    mpu9250_init();
    ak8963_init();
    bmp280_hal_init();

    /* 获取校正值 */
    sensor_get_sens(&s_accel_sens, &s_gyro_sens, mag_sens); 
    /* 计算ak8963矫正值 */
    s_ak8963_adj[0] = (0.5f * (mag_sens[0] - 128) / 128) + 1;
    s_ak8963_adj[1] = (0.5f * (mag_sens[1] - 128) / 128) + 1;
    s_ak8963_adj[2] = (0.5f * (mag_sens[2] - 128) / 128) + 1;

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
    static bool_T first_run = TRUE;


    /* 首次 启动采样 */
    if(first_run)
    {			
        s_tick = HAL_GetTick();
        s_buf_type = mpu9250_E;
        si_read_dma(MPU9250_DEV_ADDR, MPU9250_DATA_ADDR, s_buf, MPU9250_DATA_LENGTH);
        first_run = FALSE;
        return;
    }

    /* 其他 保存数据 后 启动采样 */
    if(si_read_ready()) /* 上一帧数据已达 */
    { 
        /* 解析并保存入data */
        parse();
        s_tick = HAL_GetTick();			
        if(0 == s_tick % AK8963_READ_PERIOD) /* 读AK8963周期 */
        { 
            s_buf_type = ak8963_E;
            si_read_dma(AK8963_DEV_ADDR, AK8963_DATA_FIRST_ADDR, s_buf, AK8963_DATA_LENGTH);
        }
        else if(0 == s_tick % MPU9250_READ_PERIOD) /* 读MPU9250周期 */
        { 
            s_buf_type = mpu9250_E;
            si_read_dma(MPU9250_DEV_ADDR, MPU9250_DATA_ADDR, s_buf, MPU9250_DATA_LENGTH);
        }
        else
        {
            return;
        }
    }
} 

static void parse(void)
{ 
    int16_T buf_i16[7] = {0};

    f32_T gyro[3] = {0.0f};
    f32_T accel[3] = {0.0f};
    f32_T compass[3] = {0.0f};

    s_data.type = s_buf_type;
    s_data.time = s_tick;

    if(mpu9250_E == s_buf_type)
    { 
        /* 加计 */
        buf_i16[0] = ((s_buf[0]) << 8) | s_buf[1];
        buf_i16[1] = ((s_buf[2]) << 8) | s_buf[3];
        buf_i16[2] = ((s_buf[4]) << 8) | s_buf[5];
        accel[0] = buf_i16[0] / (f32_T)s_accel_sens;
        accel[1] = buf_i16[1] / (f32_T)s_accel_sens;
        accel[2] = buf_i16[2] / (f32_T)s_accel_sens;
        s_data.val.mpu9250.accel[0] = accel[0];
        s_data.val.mpu9250.accel[1] = accel[1];
        s_data.val.mpu9250.accel[2] = accel[2];

        /* 陀螺仪 */
        buf_i16[4] = ((s_buf[8]) << 8) | s_buf[9];
        buf_i16[5] = ((s_buf[10]) << 8) | s_buf[11];
        buf_i16[6] = ((s_buf[12]) << 8) | s_buf[13];
        gyro[0] = buf_i16[4] / (f32_T)s_gyro_sens;
        gyro[1] = buf_i16[5] / (f32_T)s_gyro_sens;
        gyro[2] = buf_i16[6] / (f32_T)s_gyro_sens;
        s_data.val.mpu9250.gyro[0] = gyro[0];
        s_data.val.mpu9250.gyro[1] = gyro[1];
        s_data.val.mpu9250.gyro[2] = gyro[2];
#if 0
        /* 温度 不使用 */
        buf_i16[3] = ((buf[6]) << 8) | buf[7];
        s_data.val.mpu9250.temp = 21 + (buf_i16[3] / 321.0f);
#endif

    }
    else if(ak8963_E == s_buf_type)
    {
        s_data.val.ak8963.st1 = s_buf[0];
        s_data.val.ak8963.st2 = s_buf[7];

        if(AK8963_ST1_DRDY_BIT & s_buf[0]) /* 有效数据 */ 
        {
            buf_i16[0] = s_buf[2] << 8 | s_buf[1];
            buf_i16[1] = s_buf[4] << 8 | s_buf[3];
            buf_i16[2] = s_buf[6] << 8 | s_buf[5]; 

            compass[0] = buf_i16[0] * s_ak8963_adj[0];
            compass[1] = buf_i16[1] * s_ak8963_adj[1];
            compass[2] = buf_i16[2] * s_ak8963_adj[2]; 
            
            s_data.val.ak8963.val[0] = compass[0];
            s_data.val.ak8963.val[1] = compass[1];
            s_data.val.ak8963.val[2] = compass[2];
        } 
        else /* 数据无效 */
        {
            s_data_ready = FALSE;
            return;
        }
    }
    else
    {
        ;
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
