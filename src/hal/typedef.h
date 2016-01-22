/********************************************************************************
*
* 文件名  ： tyepdef.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150321
* 版本号  ： v1.0
* 文件描述： 通用类型定义文件
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _TYPE_DEF_H_
#define _TYPE_DEF_H_

/************************************ 头文件 ***********************************/
#include <stdint.h>

/************************************ 宏定义 ***********************************/
/* bool 宏 */
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

/* 空字符 */
#ifndef NUL
#define NUL ('\0')
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

/*********************************** 类型定义 **********************************/
typedef unsigned char       uint8_T;
typedef unsigned short      uint16_T;
typedef unsigned int        uint32_T;
typedef unsigned long long  uint64_T;
typedef char                int8_T;
typedef short               int16_T;
typedef int                 int32_T;
typedef long long           int64_T;
typedef unsigned char       bool_T;
typedef float               f32_T;
typedef void(*func_T)       (void *argv);

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/

#endif

