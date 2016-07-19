/******************************************************************************
 *
 * 文件名  ： inv.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160112
 * 版本号  ： 1.0
 * 文件描述： inv mpu驱动适配层
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"
#include "board.h"
#include "misc.h"
#include "si.h"
#include "inv.h"
#include "mpu9250.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数实现区 *********************************/
/* 目前inv_mpu中初始化相关的代码使用轮询模式 实际工作后的数据采集使用DMA模式
 * inv_mpu驱动使用的设备地址格式与si标准的不一致需要转换
 * TODO: 统一I2C读写 */
inline static unsigned char addr_convert(unsigned char addr)
{
    return (addr << 1);
}

inline int inv_read_buf(unsigned char dev_addr,
        unsigned char reg_addr,
        unsigned short buf_len, 
        unsigned char *buf)
{
    uint8_T dev_addr_real = addr_convert(dev_addr);
   
    si_read_poll(dev_addr_real, reg_addr, buf, buf_len);

    return 0;
}

inline int inv_write_buf(unsigned char dev_addr,
        unsigned char reg_addr,
        unsigned short buf_len, 
        const unsigned char *buf)
{
    uint8_T dev_addr_real = addr_convert(dev_addr);

    si_write_poll(dev_addr_real, reg_addr, buf, buf_len);

    return 0;
}

inline int inv_get_ms(unsigned long *count)
{
    *count = HAL_GetTick();
    return 0;
}

inline void inv_delay_ms(unsigned int ms)
{
    HAL_Delay(ms);
}

