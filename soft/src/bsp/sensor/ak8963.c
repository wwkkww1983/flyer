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
#include "si.h"
#include "inv_mpu.h"
#include "console.h"
#include "ak8963.h"

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
        debug_log("AK8963异常.\r\n");
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

/* 磁力计测试 */
void ak8963_test(void)
{
    misc_time_T time1, time2, time3;
    misc_time_T diff1, diff2;
    ak8963_val_T data;
    uint8_T buf[AK8963_DATA_LENGTH]; /* 避免溢出 */

    get_now(&time1);
    ak8963_read(buf);
    get_now(&time2);
    while(!si_read_ready()); 
    get_now(&time3); 
    diff_clk(&diff1, &time1, &time2);
    diff_clk(&diff2, &time2, &time3); 

    debug_log("磁力计(%dBytes)DMA读取请求耗时:%ums,%.2fus\r\n", AK8963_DATA_LENGTH, diff1.ms, 1.0f * diff1.clk / 84);
    debug_log("磁力计等待数据耗时:%ums,%.2fus\r\n", diff2.ms, 1.0f * diff2.clk / 84); 
    if(0 == ak8963_parse(&data, buf))
    { 
        debug_log("X:%7.4f,Y:%7.4f,Z:%7.4f\r\n", data.val[0], data.val[1], data.val[2]);
    }
    else
    { 
        debug_log("%s:%d磁力计未为获取到有效数据\r\n", __FILE__, __LINE__);
        while(1);
    }
    debug_log("磁力计数据:ST1:0x%02x,ST2:0x%02x\r\n", data.st1, data.st2);
} 

/* 读取一帧 */
int32_T ak8963_read(uint8_T *buf)
{
    si_read_dma(AK8963_DEV_ADDR, AK8963_DATA_FIRST_ADDR, buf, AK8963_DATA_LENGTH);
    return AK8963_DATA_LENGTH;
}

/*
 * 返回 0 成功
 * 其他   无效数据
 *
 * */
int32_T ak8963_parse(ak8963_val_T *ak8963, const uint8_T *buf)
{
    int16_T buf_i16[3] = {0};
    int16_T val_adjd[3] = {0};
    
    ak8963->st1 = buf[0];
    ak8963->st2 = buf[7];
    
    if((AK8963_ST1_DRDY_BIT & ak8963->st1)   /* 有效数据 */ 
    && !(AK8963_ST2_HOFL_BIT & ak8963->st2)) /* 未超量程溢出 */
    {

        buf_i16[0] = buf[2] << 8 | buf[1];
        buf_i16[1] = buf[4] << 8 | buf[3];
        buf_i16[2] = buf[6] << 8 | buf[5];

        /* 避免溢出,校准过程只能在这里处理 */
        val_adjd[0] = ((buf_i16[0] * s_adj[0]) >> 8) + 1;
        val_adjd[1] = ((buf_i16[1] * s_adj[1]) >> 8) + 1;
        val_adjd[2] = ((buf_i16[2] * s_adj[2]) >> 8) + 1;

        ak8963->val[0] = val_adjd[0] * AK8963_16BIT_STEP;
        ak8963->val[1] = val_adjd[1] * AK8963_16BIT_STEP;
        ak8963->val[2] = val_adjd[2] * AK8963_16BIT_STEP;

        return 0;
    } 
    else /* 数据无效 */
    {
        return -1;
    }
}

