/********************************************************************************
*
* 文件名  ： debug.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150811
* 版本号  ： v1.0
* 文件描述： 调试宏(目前使用esp8266)
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _DEBUG_H_
#define _DEBUG_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"

#include "console.h"
#include "esp8266.h"

/************************************ 宏定义 ***********************************/


/*********************************** 实现函数 **********************************/
/* DEBUG_LEVEL > 0 输出错误打印信息 */
#if (DEBUG_LEVEL > 0)
#define  err_log(...)   esp8266_printf("ERR: ") ;\
                        esp8266_printf(__VA_ARGS__);\
                        esp8266_printf("\r\n");
#else
#define err_log(...)
#endif

/* DEBUG_LEVEL > 1 输出调试打印信息 */
#if (DEBUG_LEVEL > 1)
#define debug_log(...)  esp8266_printf(__VA_ARGS__);
#else
#define debug_log(...)
#endif

/* DEBUG_LEVEL > 2 输出跟踪信息 */
#if (DEBUG_LEVEL > 2)
#define trace_log(...)  esp8266_printf("TRACE : ") ;\
                        esp8266_printf(__VA_ARGS__);\
                        esp8266_printf("\r\n");
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

/*********************************** 接口函数 **********************************/
#endif

