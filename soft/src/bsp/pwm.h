/********************************************************************************
*
* 文件名  ： pwm.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160126
* 版本号  ： v1.0
* 文件描述： pwm模块
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _PWM_H_
#define _PWM_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"

/************************************ 宏定义 ***********************************/
#define PWM_MAX_VAL             (1000U)

/*********************************** 类型定义 **********************************/
/* LED名字 编号 */
typedef enum
{
    PWM_FRONT = 0,
    PWM_RIGHT = 1,
    PWM_BACK  = 2,
    PWM_LEFT  = 3,
    PWM_MAX
}PWM_NAME;

/* pwm硬件 */
typedef struct pwm_list_tag
{
    PWM_NAME    name; /* 通道名字 */
    TIM_TypeDef *tim; /* 计时器 */
    uint32_T    ch;   /* 通道 */
}PWM_LIST_T;

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
/* 初始化 */
void pwm_init(void);
/* 设置pwm输出 */
void pwm_set(PWM_NAME pwm, uint32_T val);
/* 获取周期最值 */
uint32_T pwm_get_period(void);
/* 测试pwm */
void pwm_test(void);

#endif

