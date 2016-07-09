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
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu9250.h"
#include "exti.h"
#include "sensor.h"
#include "stm32f4xx_hal.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
__IO bool_T g_mpu_fifo_ready = FALSE;
__IO int16_T g_int_status = 0;
/********************************** 函数声明区 *********************************/
static unsigned char addr_convert(unsigned char addr);
static void run_self_test(void);
//static void int_callback(void *argv);


/********************************** 函数实现区 *********************************/
/*******************************************************************************
*
* 函数名  : mpu9250_read_buf
* 负责人  : 彭鹏
* 创建日期: 20150703
* 函数功能: i2c读取
*
* 输入参数: dev_addr 设备地址
*           reg_addr 寄存器启始地址
*           buf_len 缓存大小
*
* 输出参数: ptr_read_buf 读取的缓存
*
* 返回值  : 0       成功
*           其他    失败
*
* 调用关系: 无
* 其 它   : buf_len过大(需要的数据过多会卡死)
*
******************************************************************************/
inline int mpu9250_read_buf(unsigned char dev_addr,
        unsigned char reg_addr,
        unsigned short buf_len, 
        unsigned char *buf)
{
    uint8_T dev_addr_real = addr_convert(dev_addr);
   
    sensor_read_poll(dev_addr_real, reg_addr, buf, buf_len);

    return 0;
}

/*******************************************************************************
*
* 函数名  : mpu9250_write_buf
* 负责人  : 彭鹏
* 创建日期: 20150703
* 函数功能: i2c写入
*
* 输入参数: dev_addr 设备地址
*           reg_addr 寄存器启始地址
*           buf_len 缓存大小
*
* 输出参数: ptr_write_buf 写入的缓存
*
* 返回值  : 0       成功
*           其他    失败
*
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
inline int mpu9250_write_buf(unsigned char dev_addr,
        unsigned char reg_addr,
        unsigned short buf_len, 
        const unsigned char *buf)
{
    uint8_T dev_addr_real = addr_convert(dev_addr);

    sensor_write_poll(dev_addr_real, reg_addr, buf, buf_len);

    return 0;
}

/*******************************************************************************
*
* 函数名  : mpu9250_get_ms
* 负责人  : 彭鹏
* 创建日期: 20150703
* 函数功能: 获取系统启动后的时间ms为单位
*
* 输入参数: 无
* 输出参数: count 当前ms值地址
* 返回值  : 0 正确
*           1 出错
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
inline int mpu9250_get_ms(unsigned long *count)
{
    *count = HAL_GetTick();
    return 0;
}

/*******************************************************************************
*
* 函数名  : mpu9250_delay_ms
* 负责人  : 彭鹏
* 创建日期: 20150707
* 函数功能: 延迟 ms作为单位
*
* 输入参数: 无
* 输出参数: count 当前ms值地址
* 返回值  : 0 正确
*           1 出错
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
inline void mpu9250_delay_ms(unsigned int ms)
{
    HAL_Delay(ms);
}

/*******************************************************************************
*
* 函数名  : addr_convert
* 负责人  : 彭鹏
* 创建日期: 20150703
* 函数功能: 地址转换
*           MPU9250 7bit地址 1101000
*           stm32f4 i2c接收的地址 1101000X (X 1读 x0 写)
*           故地址需要左移1bit
*
* 输入参数: addr 待转换的地址
* 输出参数: 无
* 返回值  : 转换后的地址
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
inline static unsigned char addr_convert(unsigned char addr)
{
    return (addr << 1);
}

void mpu9250_init(void)
{
    uint8_T dev_status = 0;
    uint8_T who_am_i = 0;

    /* 测试i2c是否正常工作 */
    sensor_read_poll(MPU9250_DEV_ADDR, MPU9250_WHO_AM_I_REG_ADDR, &who_am_i, 1); 
    if(MPU9250_WHO_AM_I_REG_VALUE == who_am_i)
    {
        console_printf("MPU9250正常工作.\r\n");
    }

    console_printf("初始化MPU9250.\r\n");
    if (mpu_init(NULL) != 0)
    {
        console_printf("初始化MPU失败!\r\n");
        return;
    }

    console_printf("打开传感器.\r\n");
    if (mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL|INV_XYZ_COMPASS)!=0)
    {
        console_printf("打开传感器失败.\r\n");
        return;
    }
	
    console_printf("设置主采样率:%dHz\r\n", MPU9250_MAIN_SAMPLE_RATE);
    if(mpu_set_sample_rate(MPU9250_MAIN_SAMPLE_RATE) !=0)
    {
        console_printf("设置accel+gyro采样率失败.\r\n");
        return;
    }
    console_printf("设置磁力计采样率:%dHz\r\n", MPU9250_MAG_SAMPLE_RATE);
    if(mpu_set_compass_sample_rate(MPU9250_MAG_SAMPLE_RATE) !=0)
    {
        console_printf("设置compass采样率失败.\r\n");
        return;
    }
    console_printf("设置陀螺仪量程为:%ddeg/s.\r\n", MPU9250_GYRO_FSR);
    if (mpu_set_gyro_fsr(MPU9250_GYRO_FSR)!=0)
    {
        console_printf("设置陀螺仪量程失败.\r\n");
        return;
    }
    console_printf("设置加速度计量程:%dg.\r\n", MPU9250_ACCEL_FSR);
    if (mpu_set_accel_fsr(MPU9250_ACCEL_FSR)!=0)
    {
        console_printf("设置加速度计量程失败.\r\n");
        return;
    }
    mpu_get_power_state(&dev_status);
    console_printf("MPU9250 上电%s", dev_status? "成功!\r\n" : "失败!\r\n");

    run_self_test();

		    /* 不使用MPU9250中断 */
#if 0
    exti_set_callback(int_callback, NULL);
    console_printf("MPU9250中断设置完成.\r\n");

    /* 内部设置中断 可以用于验证中断连接MCU硬件是有效的 */
    console_printf("设置MPU FIFO.\r\n");
    if (mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL)!=0)
    {
        console_printf("设置MPU FIFO失败.\r\n");
        return;
    } 
#endif

    /*
     * 初始化 DMP:
     * 1. 注册回调函数
     * 2. 调用dmp_load_motion_driver_firmware().加载inv_mpu_dmp_motion_driver.h的
     *    DMP程序.
     * 3. 加方向矩阵加入DMP.
     * 4. 调用dmp_enable_feature()使能特性
     * 5. 调用mpu_set_dmp_state(1)启动dmp
     *
     * 不能使用的特性组合:
     * 1. DMP_FEATURE_LP_QUAT < == > DMP_FEATURE_LP_QUAT
     * 2. DMP_FEATURE_SEND_CAL_GYRO < == > DMP_FEATURE_SEND_RAW_GYRO.
     *
     */
#if 0
    dmp_load_motion_driver_firmware();
    /* dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_pdata.orientation)); */
    dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(dmp_features);
    dmp_set_fifo_rate(MPU9250_DMP_SAMPLE_RATE);
    mpu_set_dmp_state(1);
#endif

    return;
}

/*static uint8_T int_cfg = 0;
static uint8_T int_en = 0;
static uint8_T int_sta = 0;
static int16_T gyro[3];
static int16_T accel[3];
static uint8_T sensor;
static uint8_T more;
static uint32_T timestamp;
static int32_T rst = 0;
static uint32_T timestamp1;
static uint32_T timestamp2;
static int32_T times = 0;
static void int_callback(void *argv)
{
#if 0
    static int32_T rst = 0;
    if(0 == times)
    {
        timestamp1 = HAL_GetTick();
    }
	  
    rst = mpu_get_int_status(&g_int_status);
    //rst = mpu_read_reg(0x37, &int_cfg);
    ////rst = mpu_read_reg(0x38, &int_en);
    //rst = mpu_read_reg(0x3A, &int_sta);
    //rst = mpu_read_fifo(gyro, accel, &timestamp, &sensor, &more);
    //g_mpu_fifo_ready = TRUE;
    if(0x0010 & g_int_status)
    {
        timestamp2 = HAL_GetTick();
        timestamp = timestamp2 - timestamp1;
        timestamp = timestamp2 - timestamp1;
    }
//#else
    if(0 != times)
    {
 //rst = mpu_read_reg(0x37, &int_cfg);
 //rst = mpu_read_reg(0x38, &int_en);
 //rst = mpu_read_reg(0x3A, &int_sta);
        g_mpu_fifo_ready = TRUE;
    }
#endif
    times++;
		while(1);
}
*/

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

        console_printf("自检通过,设置偏移:\r\n");
        console_printf("accel: %7.4f %7.4f %7.4f\r\n",
                accel[0]/65536.f,
                accel[1]/65536.f,
                accel[2]/65536.f);
        console_printf("gyro : %7.4f %7.4f %7.4f\r\n",
                gyro[0]/65536.f,
                gyro[1]/65536.f,
                gyro[2]/65536.f);
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

