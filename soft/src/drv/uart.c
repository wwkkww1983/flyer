/******************************************************************************
 *
 * 文件名  ： uart.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160624 
 * 版本号  ： 1.0
 * 文件描述： 封装stm32f4 hal中串口驱动
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
/* 消除中文打印警告 */
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f4xx_hal.h>

#include "typedef.h"
#include "config.h"
#include "board.h"
#include "uart.h"

#include "console.h"
#include "esp8266.h"

#include "comm.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/********************************** 函数声明区 *********************************/
inline static bool_T uart_tc_locked(const drv_uart_T *uart);
inline static void uart_tc_lock(drv_uart_T *uart);
inline static void uart_tc_unlock(drv_uart_T *uart);
inline static bool_T uart_rx_locked(const drv_uart_T *uart);
inline static void uart_rx_lock(drv_uart_T *uart);
inline static void uart_rx_unlock(drv_uart_T *uart);

/********************************** 函数实现区 *********************************/
/* 串口初始化 */
void uart_init(drv_uart_T *uart)
{
    UART_HandleTypeDef  *handle = &uart->handle;

    handle->Instance          = uart->reg_base;
    handle->Init.BaudRate     = uart->baud_rate;

    /* 以下内容所有串口使用默认配置 */
    handle->Init.WordLength   = UART_WORDLENGTH_8B;
    handle->Init.StopBits     = UART_STOPBITS_1;
    handle->Init.Parity       = UART_PARITY_NONE;
    handle->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    handle->Init.Mode         = UART_MODE_TX_RX;
    handle->Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(handle) != HAL_OK)
    {
        while(1);
    } 
    
    /* 解锁保证可用 */
    uart_tc_unlock(uart);
    uart_rx_unlock(uart);
}

inline static bool_T uart_tc_locked(const drv_uart_T *uart)
{
    return uart->dma_tc_lock;
}

/* FIXME: lock unlock 原子操作 */
/* 上次传输完成 Transmit Compelete 锁(尚未完成) */
inline static void uart_tc_lock(drv_uart_T *uart)
{
    uart->dma_tc_lock = TRUE;
}

/* 上次传输完成 Transmit Compelete 解锁(完成) */
inline static void uart_tc_unlock(drv_uart_T *uart)
{
    uart->dma_tc_lock = FALSE;
}

inline static bool_T uart_rx_locked(const drv_uart_T *uart)
{
    return uart->dma_rx_lock;
}

/* FIXME: lock unlock 原子操作 */
/* 上次接收锁(尚未完成) */
inline static void uart_rx_lock(drv_uart_T *uart)
{
    uart->dma_rx_lock = TRUE;
}

/* 上次接收解锁(完成) */
inline static void uart_rx_unlock(drv_uart_T *uart)
{
    uart->dma_rx_lock = FALSE;
}

/* 串口发送 */
void uart_send(drv_uart_T *uart, uint8_T *fmt, ...)
{
    int32_T n = 0; /* printf 字符串长度 */
    uint32_T send_frame_len = 0; /* printf帧长度 */
	
    while(uart_tc_locked(uart)); /* 等待上一次传输完成 */
    uart_tc_lock(uart); /* 锁住资源 */
    
    va_list args;
    va_start(args, fmt); 
    n = vsnprintf((char *)(uart->send_buf + UART_FRAME_HEAD_SIZE), UART_LINE_BUF_SIZE, (char *)fmt, args);
    va_end(args); 
    
    /* 构造协议帧 */
    comm_frame_printf_make(&send_frame_len, uart->send_buf, n);
    if(HAL_UART_Transmit_DMA(&uart->handle, (uint8_t *)uart->send_buf, send_frame_len)!= HAL_OK)
    {
        /* 出错 */
        while(1);
    }

    return;
}

void uart_send_bytes(drv_uart_T *uart, uint8_T *buf, uint32_T n)
{
    while(uart_tc_locked(uart)); /* 等待上一次传输完成 */
    uart_tc_lock(uart); /* 锁住资源 */

    if(HAL_UART_Transmit_DMA(&uart->handle, buf, n)!= HAL_OK)
    {
        /* 出错 */
        while(1);
    }
}

void uart_recv_bytes(drv_uart_T *uart, uint8_T *buf, uint32_T n)
{
    while(uart_rx_locked(uart)); /* 等待上一次接收完成 */
    uart_rx_lock(uart); /* 锁住资源 */

    if(HAL_UART_Receive_DMA(&uart->handle, buf, n)!= HAL_OK)
    {
        /* 出错 */
        while(1);
    }
}

inline bool_T uart_frame_ready(const drv_uart_T *uart)
{
    return uart_rx_locked(uart);
}

/* FIXME: 新增串口 家分支 */
/* 参考手册中 UART DMA TX章节中描述 TC中断标志本次传输完成 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(CONSOLE_UART == huart->Instance)
    { 
        uart_tc_unlock(&g_console);
    }
    else if(ESP8266_UART == huart->Instance)
    {
        uart_tc_unlock(&g_esp8266);
    }
    else
    {
        /* 出错 未实现的串口发送完成 */
        while(1);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(CONSOLE_UART == huart->Instance)
    { 
        uart_rx_unlock(&g_console);
    }
    else if(ESP8266_UART == huart->Instance)
    {
        uart_rx_unlock(&g_esp8266);
    }
    else
    {
        /* 出错 未实现的串口发送完成 */
        while(1);
    }
}

