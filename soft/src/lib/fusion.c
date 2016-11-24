/******************************************************************************
 *
 * 文件名  ： fusion
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160709 
 * 版本号  ： v1.0
 * 文件描述： 融合算法
 * 版权说明： Copyright (c) 2000-2020 GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "config.h"
#include "typedef.h"
#include "mpu9250.h"
#include "fusion.h"

#include <math.h>


/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数声明区 *********************************/
/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/
/* 6轴(3陀螺,3加计)融合 无法融合偏航角 */
void fusion_accel(f32_T *accel)
{

    f32_T theta_r = 0.0f;   /* 融合 俯仰角 */
    f32_T phi_r = 0.0f;     /* 融合 横滚角 */

    f32_T theta_a = 0.0f;   /* 加计 俯仰角 */
    f32_T phi_a = 0.0f;     /* 加计 横滚角 */

    f32_T theta_g = 0.0f;   /* 陀螺 俯仰角 */
    f32_T phi_g = 0.0f;     /* 陀螺 横滚角 */

    f32_T euler[3] = {0.0f};
    f32_T q[4] = {0.0f};

    /* FIXME: 使用查表加速三角函数 */
    /* 加计姿态 */
    theta_a = atan2(accel[1], accel[2]);
    phi_a = atan2(accel[0], accel[2]);

    /* 获取四元数姿态(陀螺) */
    mpu9250_get_quat(q);
    math_quaternion2euler(euler, q);
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
    theta_r = theta_g + (theta_a - theta_g) * FUSION_ACCEL_THETA_RATE;
    phi_r = phi_g + (phi_a - phi_g) * FUSION_ACCEL_PHI_RATE;
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
    math_euler2quaternion(q, euler);
    mpu9250_set_quat(q);
    
#if 0
    static int32_T pp = 0;
    if(1000 == pp)
    {
        debug_log("%.4f,%.4f,%.4f\r\n", euler[0], euler[1], euler[2]);
        debug_log("%.4f,%.4f,%.4f,%.4f\r\n", q[0], q[1], q[2], q[3]);
        pp = 0;
    }
    pp++;
#endif

#if 0
    static int pp = 0;
    debug_log("%d:\n", pp);
    debug_log("%7.4f,%7.4f + ", math_arc2angle(theta_a), math_arc2angle(phi_a));
    debug_log("%7.4f,%7.4f => ", math_arc2angle(theta_g), math_arc2angle(phi_g));
    debug_log("%7.4f,%7.4f,%7.4f ", math_arc2angle(theta_r), math_arc2angle(phi_r), math_arc2angle(psi));
    debug_log("%7.4f,%7.4f,%7.4f,%7.4f\n", q[0],q[1],q[2],q[3]);
    debug_log("\n");
    if(10000 == pp)
    {
        while(1);
    }

    pp++;
#endif
}

