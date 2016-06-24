/******************************************************************************
 *
 * 文件名  ： console.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160624 
 * 版本号  ： 1.0
 * 文件描述： 控制台模块
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

#include "typedef.h"
#include "config.h"
#include "board.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
static UART_HandleTypeDef s_uart_handle;

/********************************** 函数声明区 *********************************/
static void console_printf_poll(uint8_T *buf, int32_T n);

/********************************** 函数实现区 *********************************/
/* 控制台初始化 */
void console_init(void)
{
    s_uart_handle.Instance          = CONSOLE_UART;
    s_uart_handle.Init.BaudRate     = CONSOLE_BAUDRATE;
    s_uart_handle.Init.WordLength   = UART_WORDLENGTH_8B;
    s_uart_handle.Init.StopBits     = UART_STOPBITS_1;
    s_uart_handle.Init.Parity       = UART_PARITY_NONE;
    s_uart_handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_uart_handle.Init.Mode         = UART_MODE_TX_RX;
    s_uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&s_uart_handle) != HAL_OK)
    {
        while(1);
    } 
}

void console_printf(uint8_T *fmt, ...)
{ 
    if(NULL == fmt) /* 无需打印 */
    {
        return;
    }

    char *printf_buf = malloc(CONSOLE_PRINTF_POLL_BUF_SIZE);

    va_list args;
    int32_T n = 0;

    va_start(args, fmt); 
    n = vsnprintf(printf_buf, CONSOLE_PRINTF_POLL_BUF_SIZE, (char *)fmt, args);
    va_end(args);

    /* 轮询 发送 */
    console_printf_poll((uint8_T *)printf_buf, n);

    free(printf_buf);
    printf_buf = NULL;

    return;
}

/* 控制台打印 轮询 */
static void console_printf_poll(uint8_T *buf, int32_T n)
{ 
    /* 轮询 发送 */
    if(HAL_UART_Transmit(&s_uart_handle, buf, n, n/CONSOLE_PRINTF_POLL_TIMEOUT_DIV)!= HAL_OK)
    {
        while(1);
    }

    return;
}

