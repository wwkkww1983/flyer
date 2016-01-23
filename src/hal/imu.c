/******************************************************************************
 *
 * 文件名  ： imu.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160112 
 * 版本号  ： 1.0
 * 文件描述： imu i2c 驱动程序实现
 * 版权说明： Copyright (c) GNU
 * 其    他： 实现非阻塞
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
/* 消除中文打印警告 */
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include "imu.h"
#include "mpu9250.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "board.h"
#include "misc.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* STM32F4Cube HAL驱动 */
static I2C_HandleTypeDef s_imu_handle;

/* 初始化时不调用 imu_read_in_systick */
static bool_T s_init_done = FALSE;

/* AB面 */
static data_T s_bufA[IMU_HALF_SIZE];
static data_T s_bufB[IMU_HALF_SIZE];
/* 当前缓存写入到的位置 */
static uint32_T s_used_size = 0;
/* 当前面缓存是否全部读完 */
static bool_T s_read_done = TRUE;

/* 当前imu中写入的缓存 */
static bool_T s_ab_flag = A_BUF_WRITING;
/* 完成一次中断读取 */
static bool_T s_end_flag = TRUE;
/* 本次需要读取的数据类型 */
/* systick 与 i2c中断并发访问 */
static bool_T s_type = ERR_E;

/********************************** 函数声明区 *********************************/
static void init_bufAB(data_T *data_t_buf);
static data_T *get_writing_buf(void);
static data_T *get_reading_buf(void);
static void imu_read_it(uint8_T dev_addr, uint16_T reg_addr, uint32_T n);

/********************************** 函数实现区 *********************************/
void imu_init(void)
{
    if(HAL_I2C_STATE_RESET != HAL_I2C_GetState(&s_imu_handle))
    {
        assert_failed(__FILE__, __LINE__);
    }

    s_imu_handle.Instance              = IMU_I2C;
    s_imu_handle.Init.ClockSpeed       = IMU_RATE;
    s_imu_handle.Init.DutyCycle        = I2C_DUTYCYCLE_2;
    s_imu_handle.Init.OwnAddress1      = 0;
    s_imu_handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    s_imu_handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    s_imu_handle.Init.OwnAddress2      = 0;
    s_imu_handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    s_imu_handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE; 
    
    if(HAL_OK != HAL_I2C_Init(&s_imu_handle))
    {
        assert_failed(__FILE__, __LINE__);
    } 
    
    init_bufAB(s_bufA);
    init_bufAB(s_bufB);
    
    uint32_T buf_size = sizeof(s_bufA);

    debug_log("AB 面缓存共计%.3fkByte\r\n", 2.0f * buf_size / 1024);

    return;
}

/* 初始化时采用轮询模型 */
void imu_read_poll(uint8_T dev_addr, uint16_T reg_addr, uint8_T *buf, uint32_T n)
{
    if(HAL_OK != HAL_I2C_Mem_Read(&s_imu_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    }

    return;
}

void imu_write_poll(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n)
{ 
    if(HAL_OK != HAL_I2C_Mem_Write(&s_imu_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_T *)buf, (uint16_T)(n), HAL_MAX_DELAY))
    {
        assert_failed(__FILE__, __LINE__);
    } 

    return;
}

/* 以下实际工作是采用的模型 */
static void init_bufAB(data_T *data_t_buf)
{
    /* 初始化AB面缓存 */
    int32_T i = 0;
    int32_T j = 0;
    for(i = 0; i< IMU_HALF_SIZE; i++)
    {
        data_t_buf[i].type = ERR_E;
        data_t_buf[i].time = 0;
        for(j = 0; j < DATA_TYPE_BUF_SIZE; j++)
        {
            data_t_buf[i].buf[j] = 0;
        }
    }
}

inline static void imu_read_accel_it(void)
{ 
    data_T *data = get_writing_buf(); 
    data[s_used_size].type = accel_E; 
    data[s_used_size].time = HAL_GetTick();
    imu_read_it(MPU9250_DEV_ADDR, MPU9250_ACCEL_DATA_ADDR, ACCEL_BUF_SIZE);
}

inline static void imu_read_gyro_it(void)
{
    data_T *data = get_writing_buf(); 
    data[s_used_size].type = gyro_E;
    data[s_used_size].time = HAL_GetTick();
    imu_read_it(MPU9250_DEV_ADDR, MPU9250_GYRO_DATA_ADDR, GYRO_BUF_SIZE);
}

inline static void imu_read_compass_it(void)
{
    data_T *data = get_writing_buf(); 
    data[s_used_size].type = compass_E;
    data[s_used_size].time = HAL_GetTick();
    imu_read_it(MPU9250_COMPASS_DEV_ADDR, MPU9250_COMPASS_DATA_ADDR, COMPASS_BUF_SIZE);
}

/* 以下中断 读 作为线程实体 */
static void imu_read_it(uint8_T dev_addr, uint16_T reg_addr, uint32_T n)
{
    data_T *data = get_writing_buf(); 
    uint8_T *buf = data[s_used_size].buf;

    s_used_size++; /* 注意一致性读取一个数据 */
    if(IMU_HALF_SIZE == s_used_size) /* A B 面 切换 */
    {
        if(FALSE == s_read_done)
        {
            ERR_STR("融合速率过慢,缓冲溢出.\r\n");
            while(1);
        }

        if(A_BUF_WRITING == s_ab_flag)
        {
            s_ab_flag = B_BUF_WRITING;
        }
        else
        {
            s_ab_flag = A_BUF_WRITING;
        }
        s_used_size = 0; 
        s_read_done = FALSE;
    }

    /* TODO:优化性能 不做冗余处理 仅启动中断读 */
    if(HAL_OK != HAL_I2C_Mem_Read_IT(&s_imu_handle, dev_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, buf, (uint16_T)(n)))
    {
        assert_failed(__FILE__, __LINE__);
    }

    return;
}

/* I2C读取完成回调 */
/* compass gyro accel 依次读取 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{ 
    /* 状态机 */
    switch(s_type)
    {
        case compass_E:
            {
                s_type = gyro_E;
                imu_read_gyro_it();
                break;
            }
        case gyro_E:
            {
                s_type = accel_E;
                imu_read_accel_it();
                break;
            }
        case accel_E:
            {
                s_type = ERR_E;
                s_end_flag = TRUE;
                break;
            }
        default:
            {
                ERR_STR("HAL_I2C_MemRxCpltCallback 状态机出错.\r\n");
                while(1);
            }
    }
}

/* 中断代码 */
void I2C3_EV_IRQHandler(void)
{
    /* TODO:优化性能 不做冗余处理 仅处理读完成标记 */
    HAL_I2C_EV_IRQHandler(&s_imu_handle);
}

/* imu从传感器获取数据入口在这里 */
/* compass gyro accel 依次读取 */
inline void imu_read_in_systick(void)
{ 
    if(!s_init_done)
    {
        return;
    }

    /* 上次未读完 这次不开始读数据 会照成采样率下降 */
    if(!s_end_flag)
    {
        ERR_STR("imu_read_it 调用过快,i2c速度不够.\r\n");
        while(1);
    }
    s_end_flag = FALSE;

    uint32_T tick = HAL_GetTick();

    /* compass 10ms 采样 */
    if(0 == tick % IMU_COMPASS_READ_FREQ)
    {
        s_type = compass_E;
        imu_read_compass_it();
    }
    /* gyro & accel 1ms 采样 */
    else
    {
        s_type = gyro_E;
        imu_read_gyro_it();
    }
}

/* 获取n个数据 存放于 data, 返回值为实际可读取的数据大小 */
inline uint32_T imu_read_buf(data_T **data, uint32_T n)
{ 
    static uint32_T index = 0;
    data_T *buf = NULL;

    uint32_T index_new = 0;
    uint32_T n_real = 0;

    assert_param(NULL != data); 
    if(s_read_done) /* 缓冲已经读完 */
    {
        return 0;
    }
    
    index_new = index + n;
    buf = get_reading_buf();
    *data = buf + index;

    /* 更新:n_real index s_read_done */
    if(index_new >= IMU_HALF_SIZE) /* index_new 属于 [0, IMU_HALF_SIZE) */
    {
        s_read_done = TRUE;
        n_real = IMU_HALF_SIZE - index - 1;
        index = 0;
    }
    else
    {
        index = index_new;
        n_real = n;
    }

    
    return n_real;

}

/* 与 get_reading_buf 相反 */
inline static data_T *get_writing_buf(void)
{
    /* 求AB面缓存 */
    if(A_BUF_WRITING == s_ab_flag)
    {
        return s_bufA;
    }
    else
    {
        return s_bufB;
    }
}

/* 与 get_writing_buf 相反 */
inline static data_T *get_reading_buf(void)
{
    /* 求AB面缓存 */
    if(A_BUF_WRITING == s_ab_flag)
    {
        return s_bufB;
    }
    else
    {
        return s_bufA;
    }
}

inline void imu_stop(void)
{
    s_init_done = FALSE;
}

inline void imu_start(void)
{
    s_init_done = TRUE;
}

/* 是否已经读取好第一面 */
inline void imu_init_done_wait(void)
{
    while(s_read_done);
}

