/********************************************************************************
*
* 文件名  ： pid.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150825
* 版本号  ： v1.0
* 文件描述： pid算法接口
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _PID_H_
#define _PID_H_

/************************************ 头文件 ***********************************/
#include "config.h"
#include "typedef.h"

/************************************ 宏定义 ***********************************/

/*--------------------------------- 接口声明区 --------------------------------*/
typedef struct pid_T_tag{
    f32_T kp;     /* 比例参数 */
    f32_T ki;     /* 积分参数 */
    f32_T kd;     /* 微分参数 */

    f32_T expect; /* 期望值 */
    f32_T out;    /* pid输出 */

    /* 以下值每次pid_update更新 */
    f32_T last;   /* 上次测量值 用于求微分 */
    f32_T acc;    /* 积分值 */
}PID_T;

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void pid_update(PID_T *pid, f32_T measured);

#endif

