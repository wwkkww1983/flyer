/********************************************************************************
*
* 文件名  ： ctrl.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150825
* 版本号  ： v1.0
* 文件描述： ctrl算法接口
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _CTRL_H_
#define _CTRL_H_

/************************************ 头文件 ***********************************/
#include "config.h"
#include "typedef.h"
#include "pid.h"

/************************************ 宏定义 ***********************************/
#define CTRL_THETA                       (0)
#define CTRL_PHI                         (1)
#define CTRL_PSI                         (2)
#define CTRL_EULER_MAX                   (3)

#define CTRL_THETA_KP_INIT               (0.3f)
#define CTRL_THETA_KI_INIT               (0.0f)
#define CTRL_THETA_KD_INIT               (0.0f)
#define CTRL_THETA_EXPECT_INIT           (0.0f)

#define CTRL_PHI_KP_INIT                 (0.3f)
#define CTRL_PHI_KI_INIT                 (0.0f)
#define CTRL_PHI_KD_INIT                 (0.0f)
#define CTRL_PHI_EXPECT_INIT             (0.0f)

#define CTRL_PSI_KP_INIT                 (0.3f)
#define CTRL_PSI_KI_INIT                 (0.0f)
#define CTRL_PSI_KD_INIT                 (0.0f)
#define CTRL_PSI_EXPECT_INIT             (0.0f)

/*--------------------------------- 接口声明区 --------------------------------*/
typedef struct CTRL_T_tag{
    int32_T base;
    f32_T   adj;  /* 使用浮点数 保证pid小数输出时的精度 */
}CTRL_T;

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void ctrl_init(void);
void ctrl_update(void);
void ctrl_set_pid(int32_T euler_index, const PID_T *pid);
void ctrl_set_expect(int32_T euler_index, f32_T expect);
void ctrl_get_pid_out(f32_T *out);
/* 关闭电机 */
void ctrl_motor_off(void); 
void ctrl_set_acceleralor(const int32_T *val);
void ctrl_get_acceleralor(int32_T *val, int32_T *val_max);

#endif

