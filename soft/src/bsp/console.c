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
static UART_HandleTypeDef s_uart_handle;
static char s_printf_buf[CONSOLE_PRINTF_BUF_SIZE] = {0};

/* AB面 */
static char s_a_buf[CONSOLE_PRINTF_AB_BUF_SIZE] = {0};
static char s_b_buf[CONSOLE_PRINTF_AB_BUF_SIZE] = {0};
static char s_buf_flag = 0;

/********************************** 函数声明区 *********************************/
static uint8_T console_getc_poll(void);
static void console_printf_it(uint8_T *buf, int32_T n);
//static uint8_T console_getc_it(void);

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

    /* 初始化printf缓存 */
    for(int32_T i = 0; i < CONSOLE_PRINTF_BUF_SIZE; i++) 
    {
        s_printf_buf[i] = NUL;
    }

    /* 初始化AB缓存 */
    for(int32_T i = 0; i < CONSOLE_PRINTF_AB_BUF_SIZE; i++) 
    { 
        s_a_buf[i] = NUL;
        s_b_buf[i] = NUL;
    } 
    s_buf_flag = 0;
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
    int32_T printf_length = 0; /* printf 调用需要发送的长度 */
    int32_T space_length = 0; /* 当前可写面剩余空间 */
    int32_T length = 0; /* 需要拷贝到缓存的长度 */

    va_start(args, fmt); 
    printf_length = vsnprintf(s_printf_buf, CONSOLE_PRINTF_BUF_SIZE, (char *)fmt, args);
    va_end(args);

    console_printf_it((uint8_T *)s_printf_buf, printf_length);

#if 0
    /* step1: 拷贝到后台面(初始为b),避免溢出 */
    space_length = CONSOLE_PRINTF_AB_BUF_SIZE - strlen(s_b_buf);
    length = (printf_length < space_length) ? (printf_length) : (space_length);
    strncat(s_b_buf, s_printf_buf, length);
    /* step2: */

    /* 中断发送 */
    console_printf_it((uint8_T *)s_b_buf, n);
#endif

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
    if(HAL_UART_Receive(&s_uart_handle, &c, 1, HAL_MAX_DELAY)!= HAL_OK)
    {
        while(1);
    }

    return c;
}

/************************************* 中断 ************************************/
/* 输出一个字符 */
static void console_printf_it(uint8_T *buf, int32_T n)
{
    /* 轮询 发送 */
    if(HAL_UART_Transmit_IT(&s_uart_handle, buf, n)!= HAL_OK)
    {
        while(1);
    }
}

/*********************************** 中断句柄 **********************************/
int i = 0;
void CONSOLE_UART_IRQHANDLER(void)
{
    i = 1;
    HAL_UART_IRQHandler(&s_uart_handle);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    i = 2;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    i = 3;
}
