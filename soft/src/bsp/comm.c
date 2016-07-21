/******************************************************************************
 *
 * 文件名  ： comm.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160721 
 * 版本号  ： 1.0
 * 文件描述： 交互模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "config.h"
#include "typedef.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stm32f4xx_hal.h>

#include "board.h"
#include "uart.h"
#include "esp8266.h"
#include "console.h"
#include "comm.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
const drv_uart_T *s_comm_uart = NULL;

/********************************** 函数声明区 *********************************/
static void parse(const uint8_T *frame);

/********************************** 函数实现区 *********************************/
/* 配置 协议走的通道 console or esp8266 */
void comm_init(const drv_uart_T *comm_uart)
{
    if(NULL == comm_uart)
    {
        while(1);
    }

    s_comm_uart =  comm_uart;
}

/* 通信交互任务 */
void comm_task(void)
{ 
    static bool_T first_run = TRUE; 
    static uint8_T frame_buf[COMM_DOWN_FRAME_BUF_SIZE] = {0};

    if(first_run) /* 首次运行启动串口接收 */
    {
        first_run = FALSE;
    }
    else /* 非首次运行 等待帧 & 解析帧 & 启动串口接收 */
    { 
        /* 等待上一帧接收完成 */
        if(uart_frame_ready(s_comm_uart))
        {
            /* 帧未到达 退出(等待下一轮) */
            return;
        }
        else
        { 
            /* 解析(包含处理) */
            parse(frame_buf);

            /* 等待下行帧 */
            uart_recv_bytes((drv_uart_T *)s_comm_uart, frame_buf, COMM_DOWN_FRAME_BUF_SIZE);
        }
    }
}

static void parse(const uint8_T *buf)
{
    comm_frame_T frame = {0};

    asdfasdf

}

/************************************* 中断 ************************************/
/*********************************** 中断句柄 **********************************/

