/********************************************************************************
*
* 文件名  ： exti.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160412
* 版本号  ： v1.0
* 文件描述： 外部中断模块
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _EXTI_H_
#define _EXTI_H_

/************************************ 头文件 ***********************************/
#include "stm32f4xx_hal.h"

/************************************ 宏定义 ***********************************/

/*********************************** 类型定义 **********************************/

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void exti_init(void);
void exti_set_callback(func_T callback, void *argv);

#endif
