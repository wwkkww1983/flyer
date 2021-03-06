/********************************************************************************
*
* 文件名  ： misc.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150614
* 版本号  ： v1.0
* 文件描述： 杂项函数接口
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _MISC_H_
#define _MISC_H_

/************************************ 头文件 ***********************************/
#include "config.h"
#include "typedef.h"
#include "string.h"

/************************************ 宏定义 ***********************************/

/*--------------------------------- 接口声明区 --------------------------------*/
typedef struct{
    uint32_T ms;
    uint32_T clk;
}misc_time_T;

typedef struct{
    bool_T inited; /* 是否初始化 */
    misc_time_T last_time; /* 上次调用时间 */
    misc_time_T interval_max; /* 目前最大的时间差 */
}misc_interval_max_T;

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void assert_failed(unsigned char* file, unsigned int line);
void err_loop(void);
void get_now(misc_time_T *time);
int32_T diff_clk(misc_time_T *diff, const misc_time_T *start, const misc_time_T *end);
void misc_interval_max_update(misc_interval_max_T *interval_max);

#endif

