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
#include "uart.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
 /* board.c中配置DMA需要使用 所以不能定义为static */
drv_uart_T g_console = {
    .reg_base = CONSOLE_UART,
    .baud_rate = CONSOLE_BAUDRATE,
};

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
/* 控制台初始化 */
void console_init(void)
{ 
    uart_init(&g_console);
}

/* 控制台测试 */
void console_test(void)
{
    debug_log("观察控制台输出并按照提示操作.\r\n");
}

/************************************* 中断 ************************************/
/*********************************** 中断句柄 **********************************/
void CONSOLE_UART_IRQHANDLER(void)
{
    HAL_UART_IRQHandler(&(g_console.handle));
}

void CONSOLE_UART_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler((g_console.handle).hdmatx);
}

void CONSOLE_UART_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler((g_console.handle).hdmarx);
}

