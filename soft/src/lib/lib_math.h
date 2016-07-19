/******************************************************************************
 *
 * 文件名  ： lib_math.h
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20150824 
 * 版本号  ： v1.0
 * 文件描述： 数学库头文件
 * 版权说明： Copyright (c) 2000-2020 GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _LIB_MATH_H_
#define _LIB_MATH_H_

/************************************ 头文件 ***********************************/
#include "config.h"

/************************************ 宏定义 ***********************************/
#define MATH_PI                 (3.1415926f)
#define MATH_ANGLE2ARC_RATE     (MATH_PI/180)
#define MATH_ARC2ANGLE_RATE     (180/MATH_PI)
#define MATH_PITCH              (0)
#define MATH_ROLL               (1)
#define MATH_YAW                (2)

/*********************************** 类型定义 **********************************/


/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void math_norm(f32_T *dst, const f32_T *src, int32_T dim);
f32_T math_angle2arc(f32_T x); 
f32_T math_arc2angle(f32_T x);
void math_euler2quaternion(f32_T *q, const f32_T *euler);
void math_quaternion2euler(f32_T *euler, const float *q);
void math_vector3_cross_product(f32_T *product, const f32_T *a, const f32_T *b);
void math_quaternion_cross(f32_T *rotated, const f32_T *q, const f32_T *rotate_q);

#if 0
f32_T math_inv_sqrt(f32_T x);
void math_vetor_angle(f32_T *angle, const f32_T *a, const f32_T *b);
void math_vector3_dot_product(f32_T *product, const f32_T *a, const f32_T *b);
#endif

#endif

