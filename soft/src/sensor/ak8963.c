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
#include "si.h"
#include "inv_mpu.h"
#include "ak8963.h"
#include "sensor.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
static uint16_T s_adj[3] = {0};

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
void ak8963_init(void)
{
    uint8_T val = 0;

    si_read_poll(AK8963_DEV_ADDR, AK8963_WIA_REG_ADDR, &val, 1);
    if(AK8963_WIA_REG_VAL != val)
    {
        console_printf("AK8963异常.\r\n");
        while(1);
    }

    /* 获取矫正值 */
    uint8_T data[3] = {0};
    mpu_set_bypass(1);
    data[0] = AK8963_POWER_DOWN;
    si_write_poll(AK8963_DEV_ADDR, AK8963_CNTL1_REG_ADDR, data, 1);
    HAL_Delay(1);

    data[0] = AK8963_FUSE_ACCESS;
    si_write_poll(AK8963_DEV_ADDR, AK8963_CNTL1_REG_ADDR, data, 1);
    HAL_Delay(1);

    si_read_poll(AK8963_DEV_ADDR, AK8963_ASAX_REG_ADDR, data, 3);
    s_adj[0] = data[0] - 128;
    s_adj[1] = data[1] - 128;
    s_adj[2] = data[2] - 128;

    data[0] = AK8963_POWER_DOWN;
    si_write_poll(AK8963_DEV_ADDR, AK8963_CNTL1_REG_ADDR, data, 1);
    HAL_Delay(1);

    /* 配置为 16bit采样 + 连续工作模式 8Hz */
    val = AK8963_16BITS | AK8963_MODE_C1;
    si_write_poll(AK8963_DEV_ADDR, AK8963_CNTL1_REG_ADDR, &val, 1); 
}

void ak8963_test(void)
{
    ;
} 


/*
 * 返回 0 成功
 * 其他   无效数据
 *
 * */
int32_T ak8963_data_parse(f32_T *val, const uint8_T *buf)
{
    uint8_T st1 = 0;
    uint8_T st2 = 0;
    int16_T buf_i16[3] = {0};
    int16_T val_adjd[3] = {0};
    
    st1 = buf[0];
    st2 = buf[7]; 
    
    if((AK8963_ST1_DRDY_BIT & st1)   /* 有效数据 */ 
    && !(AK8963_ST2_HOFL_BIT & st2)) /* 未超量程溢出 */
    {
        buf_i16[0] = buf[2] << 8 | buf[1];
        buf_i16[1] = buf[4] << 8 | buf[3];
        buf_i16[2] = buf[6] << 8 | buf[5];

        /* 避免溢出,校准过程只能在这里处理 */
        val_adjd[0] = ((buf_i16[0] * s_adj[0]) >> 8) + 1;
        val_adjd[1] = ((buf_i16[1] * s_adj[1]) >> 8) + 1;
        val_adjd[2] = ((buf_i16[2] * s_adj[2]) >> 8) + 1;

        val[0] = val_adjd[0] * AK8963_16BIT_STEP;
        val[1] = val_adjd[1] * AK8963_16BIT_STEP;
        val[2] = val_adjd[2] * AK8963_16BIT_STEP;

        return 0;
    } 
    else /* 数据无效 */
    {
        return -1;
    }
}

/* 计算采样延迟 */
int32_T ak8963_sample_period(int32_T read_times)
{
    uint32_T ms_start = 0;
    uint32_T ms_end = 0;
    uint32_T times = 0;
    uint8_T  buf[AK8963_DATA_LENGTH] = {0};
    uint32_T sample_period = 0; 
    f32_T compass[3] = {0.0f};

    ms_start = HAL_GetTick();
    while(1)
    { 
        /* 从ST1读到ST2 DMA不耗CPU */
        si_read_poll(AK8963_DEV_ADDR, AK8963_DATA_FIRST_ADDR, buf, AK8963_DATA_LENGTH);

        if(0 == ak8963_data_parse(compass, buf))
        {
            debug_log("test%02d times:", times);
            debug_log("ST1:0x%02x,", buf[0]);
            debug_log("ST2:0x%02x,", buf[7]);
            
            /* 有效数据 */
            console_printf("X:%7.4f,", compass[0]);
            console_printf("Y:%7.4f,", compass[1]);
            console_printf("Z:%7.4f\r\n", compass[1]);

            times++;
            if(times > read_times)
            {
                break;
            }
        }
        else
        {
            HAL_Delay(3);
        }
    }

    ms_end = HAL_GetTick(); 
    debug_log("%7.4fms/包(%dBytes)\r\n", 1.0f * (ms_end - ms_start) / times, AK8963_DATA_LENGTH);

    sample_period = ((ms_end - ms_start) / times);
    sample_period += 1; /* 留有余地 */

    return sample_period;
}

