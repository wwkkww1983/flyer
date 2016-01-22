/******************************************************************************
 *
 * 文件名  ： lib_math.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20150824 
 * 版本号  ： v1.0
 * 文件描述： 数学库
 * 版权说明： Copyright (c) 2000-2020 GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/


/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <math.h>
#include "lib_math.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/


/********************************** 变量实现区 *********************************/


/********************************** 函数实现区 *********************************/
/*******************************************************************************
 *
 * 函数名  : math_inv_sqrt
 * 负责人  : 彭鹏
 * 创建日期: 20150729
 * 函数功能: 求平方根倒数 
 *           快速算法
 *
 * 输入参数: 待求的值
 *
 * 输出参数: 无
 *
 * 返回值:   x 的平方根倒数
 *          -1 出错
 * 调用关系: 无
 * 其 它:    http://www.matrix67.com/data/InvSqrt.pdf
 *           参数不得为零
 *
 ******************************************************************************/
inline f32_T math_inv_sqrt(f32_T x)
{
    f32_T xhalf = 0.5f * x;
    int32_T i = *(int32_T*)&x;          // get bits for f32_Ting value
    i =  0x5f375a86 - (i>>1);   // gives initial guess
    x = *(f32_T*)&i;            // convert bits back to f32_T
    x = x * (1.5f - xhalf*x*x); // Newton step
    return x;
}

/* 归一化 */
inline void math_norm(f32_T *dst, const f32_T *src, int32_T dim)
{
    f32_T square_sum = 0.0f; 
    f32_T norm = 0.0f; 
    int32_T i = 0;
    
    for(i=0;i<dim;i++)
    {
        square_sum += (src[i] * src[i]);
    } 
    
    norm = math_inv_sqrt(square_sum);
    if(0 == norm)
    {
        while(1);
    }

    for(i=0;i<dim;i++)
    {
        dst[i] = src[i] * norm;
    } 
}

/*******************************************************************************
 *
 * 函数名  : math_angle2arc
 * 负责人  : 彭鹏
 * 创建日期: 20150729
 * 函数功能: 角度转弧度
 *
 * 输入参数: 待转的角度
 * 输出参数: 无
 *
 * 返回值:   弧度
 * 调用关系: 无
 * 其 它:    x 为 0 ~ 180
 *
 ******************************************************************************/
inline f32_T math_angle2arc(f32_T x)
{
    return MATH_ANGLE2ARC_RATE * x;
}

/*******************************************************************************
 *
 * 函数名  : math_arc2angle
 * 负责人  : 彭鹏
 * 创建日期: 20150729
 * 函数功能: 弧度转角度
 *
 * 输入参数: 待转的弧度
 * 输出参数: 无
 *
 * 返回值:   角度
 * 调用关系: 无
 * 其 它:    x 为 -pi ~ pi
 *
 ******************************************************************************/
inline f32_T math_arc2angle(f32_T x)
{
    return MATH_ARC2ANGLE_RATE * x;
}

/*******************************************************************************
 *
 * 函数名  : math_vector3_cross_product
 * 负责人  : 彭鹏
 * 创建日期: 20150731
 * 函数功能: 向量叉乘
 *
 * 输入参数: a 向量a
 *           b 向量b
 *
 * 输出参数: product 叉乘值
 *
 * 返回值:    0 成功
 *           -1 失败
 * 调用关系: 无
 * 其 它:    顺序不可反
 *
 ******************************************************************************/
inline void math_vector3_cross_product(f32_T *product, const f32_T *a, const f32_T *b)
{

    /*
     * 行列式定义
     * |   i     j    k  |
     * | a[0]  a[1] a[2] |
     * | b[0]  b[1] b[2] |
     *
     *
     * = a[1]*b[2]*i + a[2]*b[0]*j + a[0]*b[1]*k
     *  -a[2]*b[1]*i - a[0]*b[2]*j - a[1]*b[0]*k
     *
     */
    product[0] = a[1] * b[2] - a[2] * b[1];
    product[1] = a[2] * b[0] - a[0] * b[2];
    product[2] = a[0] * b[1] - a[1] * b[0];
}

/* 点乘值 */
inline void math_vector3_dot_product(f32_T *product, const f32_T *a, const f32_T *b)
{
    *product = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/* 求向量夹角 */
void math_vetor_angle(f32_T *angle, const f32_T *a, const f32_T *b)
{
    f32_T a_norm[MATH_THREE] = {0.0f};
    f32_T b_norm[MATH_THREE] = {0.0f};
    f32_T cross_product[MATH_THREE] = {0.0f};
    f32_T dot_product = 0.0f;
    f32_T rst_angle = 0.0f;

    math_norm(a_norm, a, 3);
    math_norm(b_norm, b, 3);
    math_vector3_dot_product(&dot_product, a_norm, b_norm); 
    math_vector3_cross_product(cross_product, a_norm, b_norm); 
    
    rst_angle = acosf(dot_product);
    if(cross_product[2] < 0.0f) /* 外积决定符号 */
    {
        rst_angle = -rst_angle;
    }

    *angle = rst_angle;
}

void math_euler2quaternion(f32_T *q, const f32_T *euler)
{
    f32_T half_theta = 0.0f;
    f32_T half_phi = 0.0f;
    f32_T half_psi = 0.0f;

    half_theta = euler[0] / 2;
    half_phi = euler[1] / 2;
    half_psi = euler[2] / 2;

    q[0] = cos(half_theta)*cos(half_phi)*cos(half_psi) + sin(half_theta)*sin(half_phi)*sin(half_psi);
    q[1] = sin(half_theta)*cos(half_phi)*cos(half_psi) - cos(half_theta)*sin(half_phi)*sin(half_psi);
    q[2] = cos(half_theta)*sin(half_phi)*cos(half_psi) + sin(half_theta)*cos(half_phi)*sin(half_psi);
    q[3] = cos(half_theta)*cos(half_phi)*sin(half_psi) - sin(half_theta)*sin(half_phi)*cos(half_psi); 

}

void math_quaternion2euler(f32_T *euler, const float *q)
{
    f32_T theta = 0.0f;
    f32_T phi = 0.0f;
    f32_T psi = 0.0f;

    theta = atan2(q[2]*q[3] + q[0]*q[1], q[0]*q[0] + q[3]*q[3] - 0.5f);
    phi  = -asin(2*(q[1]*q[3] - q[0]*q[2]));
    psi   = atan2(q[1]*q[2] + q[0]*q[3], q[0]*q[0] + q[1]*q[1] - 0.5f);

    euler[0] = theta;
    euler[1] = phi;
    euler[2] = psi;

    /* FIXME: 是否是全姿态的，反三角函数计算出的角度是否需要修正? */
}

