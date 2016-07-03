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

/************************************ 头文件 ***********************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stm32f4xx_hal.h>

#include "typedef.h"
#include "config.h"
#include "board.h"
#include "uart.h"

#include "console.h"
#include "esp8266.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/********************************** 函数声明区 *********************************/

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
}

/* 上次传输完成 Transmit Compelete 锁住 */
inline static bool_T uart_tc_locked(drv_uart_T *uart)
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
inline void uart_tc_unlock(drv_uart_T *uart)
{
    uart->dma_tc_lock = FALSE;
}

/* 串口发送 */
void uart_send(drv_uart_T *uart, uint8_T *fmt, ...)
{
    int32_T n = 0; /* printf 调用需要发送的长度 */
    static char s_printf_buf[UART_LINE_BUF_SIZE] = {0}; /* CONSOLE缓冲大小 */

    while(uart_tc_locked(uart)); /* 等待上一次传输完成 */
    uart_tc_lock(uart); /* 锁住资源 */
    
    va_list args;
    va_start(args, fmt); 
    n = vsnprintf(s_printf_buf, UART_LINE_BUF_SIZE, (char *)fmt, args);
    va_end(args); 

    assert_param( n < UART_LINE_BUF_SIZE );

    if(HAL_UART_Transmit_DMA(&uart->handle, (uint8_t *)s_printf_buf, n)!= HAL_OK)
    {
        /* 出错 */
        while(1);
    }

    return;
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

