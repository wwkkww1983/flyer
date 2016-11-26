/******************************************************************************
 *
 * 文件名  ： filter.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20151124 
 * 版本号  ： v1.0
 * 文件描述： 滤波算法实现
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "config.h"

#include <math.h>

#include "debug.h"
#include "filter.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
/************************************* 滤波 ************************************/
/* 均值滤波 */
bool_T filter_average_3d(f32_T *filtered_val, const f32_T *orig_val)
{
    static int32_T sample_needed = FILTER_ACCEL_AVERAGE_NUMS; /* 求均值需要的样点数 */
    static f32_T orig_sum[3] = {0.0f};

    bool_T averaged_ok = FALSE;
    int32_T i = 0;

    if(sample_needed > 0) /* 求和 */
    { 
        for(i = 0; i < 3; i++)
        {
            orig_sum[i] += orig_val[i];
        }

        sample_needed--;
        averaged_ok = FALSE;
    }
    else /* 可以求均值 */
    {
        /* 求均值 */
        for(i = 0; i < 3; i++)
        {
            filtered_val[i] = orig_sum[i] / FILTER_ACCEL_AVERAGE_NUMS;
            orig_sum[i] = 0.0f; /* 清零用于下轮求和 */
        }

        /* 准备下次求均值 */
        sample_needed = FILTER_ACCEL_AVERAGE_NUMS;
        averaged_ok = TRUE;
    }

    return averaged_ok;
}

void filter_1factorial_3d(f32_T *filtered_val, const f32_T *orig_val)
{
    int32_T i = 0;
    static bool_T s_initted = FALSE;
    static f32_T s_last_val[] = FILTER_ACCEL_INIT_VAL;

    /* 初始化 */
    if(s_initted) /* 滤波 */
    {
        for(i = 0; i < 3; i++)
        {
            /*
             * s_last_val[i] = FILTER_ACCEL_1FACTOR_LAST_RATE * s_last_val[i] + (1.0f - FILTER_ACCEL_1FACTOR_LAST_RATE) * orig_val[i];
             * 等效于下式:
             * */
            s_last_val[i] = FILTER_ACCEL_1FACTOR_LAST_RATE * (s_last_val[i] - orig_val[i]) + orig_val[i];
        }
    }
    else /* 首次输出 {0,0,1}(s_last_val初值) */
    {
        s_initted = TRUE; 

    } 
    
    /* 输出当前值 */
    for(i = 0; i < 3; i++)
    {
        filtered_val[i] = s_last_val[i];
    }
}

/*********************************** 融合滤波 **********************************/
/* 6轴(3陀螺,3加计)融合 无法融合偏航角 */
void filter_fusion_accel2gyro(f32_T *quat_fusioned, const f32_T *quat, const f32_T *accel)
{
    f32_T theta_r = 0.0f;   /* 融合 俯仰角 */
    f32_T phi_r = 0.0f;     /* 融合 横滚角 */

    f32_T theta_a = 0.0f;   /* 加计 俯仰角 */
    f32_T phi_a = 0.0f;     /* 加计 横滚角 */

    f32_T theta_g = 0.0f;   /* 陀螺 俯仰角 */
    f32_T phi_g = 0.0f;     /* 陀螺 横滚角 */

    f32_T euler[EULER_MAX] = {0.0f};

    /* FIXME: 使用查表加速三角函数 */
    /* 加计姿态 */
    theta_a = atan2(accel[1], accel[2]);
    phi_a = atan2(accel[0], accel[2]);

    /* 获取四元数姿态(陀螺) */
    math_quaternion2euler(euler, quat);
    /* 陀螺姿态 */
    theta_g = euler[0];
    phi_g = euler[1];

    /* 融合 */
    /*
     * theta_r = (1 - FUSION_ACCEL_THETA_RATE) * theta_g + FUSION_ACCEL_THETA_RATE * theta_a;
     * phi_r = (1 - FUSION_ACCEL_PHI_RATE) * phi_g + FUSION_ACCEL_PHI_RATE * phi_a;
     * 优化为下式 */
#if 1
    /* 欠补偿 */
    theta_r = theta_g + (theta_a - theta_g) * FILTER_FUSION_ACCEL_THETA_RATE;
    phi_r = phi_g + (phi_a - phi_g) * FILTER_FUSION_ACCEL_PHI_RATE;
#else
    /* 过补偿 */
    theta_r = theta_a + (theta_a - theta_g) * FUSION_ACCEL_THETA_RATE;
    phi_r = phi_a + (phi_a - phi_g) * FUSION_ACCEL_PHI_RATE;
#endif


    euler[0] = theta_r;
    euler[1] = phi_r;

    /* 偏航角无法融合 */
#if 0
    euler[2] = euler[2]; 
#endif

    /* 反推回四元数 */
    math_euler2quaternion(quat_fusioned, euler);
}

