/******************************************************************************
 *
 * 文件名  ： console.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160624 
 * 版本号  ： 1.0
 * 文件描述： 控制台模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 输入 逐个字符
 *            输出 按行
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
UART_HandleTypeDef g_uart_handle; /* board.c中配置DMA需要使用 */
static char s_printf_buf[CONSOLE_PRINTF_BUF_SIZE] = {0};
/********************************** 函数声明区 *********************************/
static uint8_T console_getc_poll(void);

/********************************** 函数实现区 *********************************/
/* 控制台初始化 */
void console_init(void)
{
    g_uart_handle.Instance          = CONSOLE_UART;
    g_uart_handle.Init.BaudRate     = CONSOLE_BAUDRATE;
    g_uart_handle.Init.WordLength   = UART_WORDLENGTH_8B;
    g_uart_handle.Init.StopBits     = UART_STOPBITS_1;
    g_uart_handle.Init.Parity       = UART_PARITY_NONE;
    g_uart_handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    g_uart_handle.Init.Mode         = UART_MODE_TX_RX;
    g_uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&g_uart_handle) != HAL_OK)
    {
        while(1);
    } 
}

/*********************************** 包裹函数 **********************************/
/* 控制台打印 */
void console_printf(uint8_T *fmt, ...)
{ 
    if(NULL == fmt) /* 无需打印 */
    {
        return;
    }

    va_list args;
    int32_T n = 0; /* printf 调用需要发送的长度 */

    va_start(args, fmt); 
    n = vsnprintf(s_printf_buf, CONSOLE_PRINTF_BUF_SIZE, (char *)fmt, args);
    va_end(args); 
    
    if(HAL_UART_Transmit_DMA(&g_uart_handle, (uint8_t *)s_printf_buf, n)!= HAL_OK)
    {
        while(1);
    }

    return;
}

/* 输入字符 */
uint8_T console_getc(void)
{ 
    return console_getc_poll();
}

/*********************************** 实现函数 **********************************/
/************************************* 轮询 ************************************/
/* 输入一个字符 */
static uint8_T console_getc_poll(void)
{
    uint8_T c = 0;
    /* 阻塞 输入(读取) */
    if(HAL_UART_Receive(&g_uart_handle, &c, 1, HAL_MAX_DELAY)!= HAL_OK)
    {
        while(1);
    }

    return c;
}

/************************************* 中断 ************************************/
/*********************************** 中断句柄 **********************************/
int i = 0;
void CONSOLE_UART_IRQHANDLER(void)
{
    i = 1;
    HAL_UART_IRQHandler(&g_uart_handle);
}

void CONSOLE_UART_DMA_TX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(g_uart_handle.hdmatx);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    i = 2;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    i = 3;
}


