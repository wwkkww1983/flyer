/********************************************************************************
*
* 文件名  ： si.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160112
* 版本号  ： v1.0
* 文件描述： sensor i2 驱动头文件
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _SI_H_
#define _SI_H_

/************************************ 头文件 ***********************************/
#include <stm32f4xx_hal.h>
#include "typedef.h"
#include "config.h"

/************************************ 宏定义 ***********************************/

/*********************************** 类型定义 **********************************/
/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/
/* board.c使用 */
extern I2C_HandleTypeDef g_si_handle;;

/* main.c测试使用 */
extern bool_T g_tx_cplt;

/*********************************** 接口函数 **********************************/
void si_init(void);
void si_read_poll(uint8_T dev_addr, uint16_T reg_addr, uint8_T *buf, uint32_T n);
void si_write_poll(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n);
void si_read_dma(uint8_T dev_addr, uint16_T reg_addr, const uint8_T *buf, uint32_T n);

void si_test_poll_rate(int32_T times);
void si_test_dma_rate(void);

#endif

