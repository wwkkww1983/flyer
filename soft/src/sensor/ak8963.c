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
#include "ak8963.h"
#include "sensor.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
void ak8963_init(void)
{
    uint8_T val = 0;

    si_read_poll(AK8963_DEV_ADDR, AK8963_WIA_REG_ADDR, &val, 1);
    if(AK8963_WIA_REG_VAL == val)
    {
        console_printf("AK8963正常工作.\r\n");
    }

    /* 配置为 16bit采样 + 连续工作模式 8Hz */
    val = AK8963_16BITS | AK8963_MODE_C1;
    si_write_poll(AK8963_DEV_ADDR, AK8963_CNTL1_REG_ADDR, &val, 1);
}

void ak8963_test(void)
{
    ;
}

/* 计算采样延迟 */
int32_T ak8963_sample_period(int32_T read_times)
{
    uint32_T ms_start = 0;
    uint32_T ms_end = 0;
    uint32_T times = 0;
    uint8_T  buf[AK8963_REG_NUMS] = {0};
    uint32_T sample_period = 0; 
    int16_T mag_sens[3] = {0};
    
    /* 获取校正值 */
    sensor_get_sens(NULL, NULL, mag_sens);

    ms_start = HAL_GetTick();
    while(1)
    { 
        /* 从ST1读到ST2 DMA不耗CPU */
        si_read_poll(AK8963_DEV_ADDR, AK8963_DATA_FIRST_ADDR, buf, AK8963_DATA_LENGTH);
        if(AK8963_ST1_DRDY_BIT & buf[0]) /* 有效数据 */
        { 
            if(AK8963_ST1_DOR_BIT & buf[0])
            {
                debug_log("AK8963 有测量值跳过.\r\n");
                continue;
            }
            if(AK8963_ST2_HOFL_BIT & buf[7])
            {
                debug_log("AK8963 溢出.\r\n");
                continue;
            } 

            debug_log("test %d times:", times);
            debug_log("ST1:0x%02x,", buf[0]);
            debug_log("ST2:0x%02x,", buf[7]);
            /* 有效数据 */
            debug_log("X:0x%02x%02x,", buf[2], buf[1]);
            debug_log("Y:0x%02x%02x,", buf[4], buf[3]);
            debug_log("Z:0x%02x%02x\r\n", buf[6], buf[5]);

            debug_log("X:%7.4f,", 
                    (((long)((buf[2] << 8) | buf[1]) * mag_sens[0] ) >> 8) * 0.15f);
            debug_log("Y:%7.4f,", 
                    (((long)((buf[4] << 8) | buf[3]) * mag_sens[1] ) >> 8) * 0.15f);
            debug_log("Z:%7.4f\r\n", 
                    (((long)((buf[6] << 8) | buf[5]) * mag_sens[2] ) >> 8) * 0.15f);

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
    debug_log("%7.4fms/data\r\n", 1.0f * (ms_end - ms_start) / times);

    sample_period = ((ms_end - ms_start) / times);
    sample_period += 1; /* 留有余地 */

    return sample_period;
}

