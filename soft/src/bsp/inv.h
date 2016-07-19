/********************************************************************************
*
* 文件名  ： inv.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160112
* 版本号  ： v1.0
* 文件描述： inv mpu驱动适配层
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _INV_H_
#define _INV_H_

/************************************ 头文件 ***********************************/
#include <stm32f4xx_hal.h>
#include "typedef.h"
#include "config.h"
#include "console.h"

/************************************ 宏定义 ***********************************/

/*********************************** 类型定义 **********************************/
/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
#define inv_log_i   debug_log
#define inv_log_e   err_log
int inv_read_buf(unsigned char dev_addr, unsigned char reg_addr,
        unsigned short buf_len, unsigned char *ptr_read_buf);
int inv_write_buf(unsigned char dev_addr, unsigned char reg_addr, 
        unsigned short buf_len, const unsigned char *ptr_write_buf);
int inv_get_ms(unsigned long *count);
void inv_delay_ms(unsigned int ms);

#endif

