/******************************************************************************
 *
 * 文件名  ： misc.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20150614 
 * 版本号  ： v1.0
 * 文件描述： 实现无处可放的函数
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "misc.h"
#include "console.h"
/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
/*******************************************************************************
*
* 函数名  : assert_failed
* 负责人  : 彭鹏
* 创建日期: 20151218
* 函数功能: 断言
*
* 输入参数: file 出错源文件名字
*           line 出错源文件行数
*
* 输出参数: 无
* 返回值  : 无
* 调用关系: 无
* 其 它   : 供STM32F4Cube4 使用
*
******************************************************************************/
void assert_failed(unsigned char* file, unsigned int line)
{
    /* 会出故障 */
    /* console_printf("%s,%d, assert_failed.\r\n", file, line); */
    while(1);
}

/*******************************************************************************
*
* 函数名  : err_loop
* 负责人  : 彭鹏
* 创建日期: 20151117
* 函数功能: 任务初始化的出口函数
*
* 输入参数: 无
* 输出参数: 无
*
* 返回值  : 无
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void err_loop(void)
{
  while(1);
}

