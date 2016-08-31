/********************************************************************************
*
* 文件名  ： uart.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160624
* 版本号  ： v1.0
* 文件描述： stm32f4 hal驱动封装
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _UART_H_
#define _UART_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"

#include <stm32f4xx_hal.h>

/************************************ 宏定义 ***********************************/ 
/* 由于console/esp8266提取了公用模块到uart导致层级结构被破坏,comm.h与uart.h不能相互include */
/* 
 * printf帧头长: 12(type+len + time)
 * */
#define UART_FRAME_HEAD_SIZE                ((uint32_T)(12))
/* 
 * 16(type+len+crc + time)
 * */
#define UART_FRAME_HEAD_AND_TAIL_SIZE       ((uint32_T)(UART_FRAME_HEAD_SIZE + 4))
/* 
 * printf上行帧最大帧长: 16(type+len+crc + time) + UART_SEND_BUF_SIZE
 * */
#define UART_SEND_BUF_SIZE                  ((uint32_T)(UART_FRAME_HEAD_AND_TAIL_SIZE + UART_LINE_BUF_SIZE))

/*********************************** 类型定义 **********************************/
typedef struct drv_uart_T_tag{
    UART_HandleTypeDef  handle;     /* stm32 halt uart句柄 */
    USART_TypeDef      *reg_base;   /* 处理器uart 寄存器地址句柄 */
    int32_T             baud_rate;  /* 波特率 */	

    uint8_T             send_buf[UART_SEND_BUF_SIZE]; /* 发送缓冲 */
    __IO bool_T         dma_tc_lock;/* dma 传输完成 锁定 */
    __IO bool_T         dma_rx_lock;/* dma 接收完成 锁定 */
}drv_uart_T;

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void uart_init(drv_uart_T *uart);

/* 串口字节流裸传 */
void uart_send_bytes(drv_uart_T *uart, uint8_T *buf, uint32_T n);
void uart_recv_bytes(drv_uart_T *uart, uint8_T *buf, uint32_T n);

/* 轮询模式 */
void uart_send_bytes_poll(drv_uart_T *uart, uint8_T *buf, uint32_T n);
void uart_recv_bytes_poll(drv_uart_T *uart, uint8_T *buf, uint32_T n);

/* 供comm模块判断是否到达一帧 */
bool_T uart_frame_ready(const drv_uart_T *uart);

/* 专用于printf */
void uart_send(drv_uart_T *uart, uint8_T *fmt, ...);

#endif

