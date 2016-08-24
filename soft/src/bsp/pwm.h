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
#define PWM_ADJ_MAX_RATE        (0.50f)
#define PWM_STEP                (50)

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
    PWM_NAME    name;       /* 通道名字 */
    TIM_TypeDef *tim;       /* 计时器 */
    uint32_T    ch;         /* 通道 */

    int32_T     base;       /* 基础值 表示该桨油门 */
    int32_T     adj_val;    /* 调整值 用于控制姿态 */
    int32_T     adj_step;   /* 调整值的步长,计算adj时使用的步长 与抖动成正比与调整速度成反比 */
}PWM_LIST_T;

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
/* 初始化 */
void pwm_init(void);
/* 姿态控制 */
void pwm_update(void);
/* 获取PWM周期 */
int32_T pwm_get_period(void); 
/* 停止PWM周期 */
void pwm_stop(void);
/* 启动PWM周期 */
void pwm_start(void);

/* 油门设置:未加入adj_val, 0 <= val <= 1000 */
void pwm_set_acceleralor(const int32_T *val);
/* 油门获取:加入了adj_val, 0 <= val <= s_period(1000) */
void pwm_get_acceleralor(int32_T *val);

#endif

