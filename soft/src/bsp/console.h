/********************************************************************************
*
* 文件名  ： console.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150614
* 版本号  ： v1.0
* 文件描述： 控制台模块
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

/************************************ 头文件 ***********************************/

/************************************ 宏定义 ***********************************/
#define  err_log(...)   console_printf("ERR: ") ;\
                        console_printf(__VA_ARGS__);\
                        console_printf("\r\n");

/* DEBUG_LEVEL > 0 输出基础打印信息 */
#if (DEBUG_LEVEL > 0)
#define debug_log(...)  console_printf(__VA_ARGS__);
#else
#define debug_log(...)
#endif

/* DEBUG_LEVEL > 1 输出跟踪信息 */
#if (DEBUG_LEVEL > 1)
#define trace_log(...)  console_printf("TRACE : ") ;\
                        console_printf(__VA_ARGS__);\
                        console_printf("\r\n");
#else
#define trace_log(...)
#endif

#define TRACE_FUNC_IN   trace_log("[IN]  %-8s:%-8d%-20s", __FILE__, __LINE__, __func__)
#define TRACE_FUNC_OUT  trace_log("[OUT] %-8s:%-8d%-20s", __FILE__, __LINE__, __func__)
#define TRACE_STR(str)  trace_log("[TRC] %-8s:%-8d%-20s\t\t%s" , __FILE__, __LINE__, __func__, str)

#define ERR_STR(str)    err_log("%-8s%-8d%-20s\t\t%s" , __FILE__, __LINE__, __func__, str)

/*********************************** 类型定义 **********************************/
/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/
extern UART_HandleTypeDef g_uart_handle; /* board.c中配置DMA需要使用 */

/*********************************** 接口函数 **********************************/
void console_init(void);
void console_printf(uint8_T *fmt, ...);
void console_putc(uint8_T c);
uint8_T console_getc(void);

#endif

