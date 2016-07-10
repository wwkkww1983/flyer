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
#include "math.h"
#include "misc.h"
#include "fusion.h"
#include "lib_math.h"
#include "sensor.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
static f32_T s_quaternion[4] = {0.0f}; /* 四元数 q0 q1 q2 q3*/

/* 存储最近的accel值 磁力计融合 */
static f32_T s_accel[3] = {0.0f};


/********************************** 函数声明区 *********************************/
static void set_quaternion(f32_T *q);
static void mpu9250_gyro_fusion(uint32_T time, f32_T *gyro);
static void mpu9250_accel_fusion(uint32_T time, f32_T *accel);
static void ak8963_fusion(uint32_T time, f32_T *val, uint8_t st1, uint8_t st2);
/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/
/* 融合初始化 */
void fusion_init(void)
{
    f32_T e[3] = {0.0f}; /* 欧拉角 全零 */
    f32_T q[4] = {0.0f}; /* 四元数 */

    math_euler2quaternion(q, e);
    set_quaternion(q);

    console_printf("姿态角始值为  :%7.4f, %7.4f, %7.4f\r\n", math_arc2angle(e[0]), math_arc2angle(e[1]), math_arc2angle(e[2]));
    console_printf("四元数初始值为:%7.4f, %7.4f, %7.4f, %7.4f\r\n", q[0], q[1], q[2], q[3]);
}

/* 10ms的数据作为单位计算 */
/* 可优化为1ms融合gyro accel, 10ms融合 compass */
void fusion(void)
{
    sensor_data_T *data;

    /* step1: 确保数据获取已经获取 */
    if(!sensor_data_ready())
    {
        return;
    }

    /* step2: 融合 */ 
    sensor_get_data(&data);
    if(mpu9250_E == data->type)
    {
        mpu9250_gyro_fusion(data->time, (f32_T *)(data->val.mpu9250.gyro));
        mpu9250_accel_fusion(data->time, (f32_T *)(data->val.mpu9250.accel));
    }
    else if(ak8963_E == data->type)
    {
        ak8963_fusion(data->time, (f32_T *)(data->val.ak8963.val), (data->val.ak8963.st1), (data->val.ak8963.st2));
    }
    else
    {
        while(1);
    }

}

/* 融合算法测试 */
void fusion_test(void)
{
    ;
}

inline static void set_quaternion(f32_T *q)
{
    s_quaternion[0] = q[0];
    s_quaternion[1] = q[1];
    s_quaternion[2] = q[2];
    s_quaternion[3] = q[3];
}

inline void get_quaternion(f32_T *q)
{ 
    q[0] = s_quaternion[0];
    q[1] = s_quaternion[1];
    q[2] = s_quaternion[2];
    q[3] = s_quaternion[3];
}

static void mpu9250_gyro_fusion(uint32_T time, f32_T *gyro)
{
    f32_T wx = gyro[0];
    f32_T wy = gyro[1];
    f32_T wz = gyro[2];

    f32_T q[4] = {0.0f};
    f32_T q_norm[4] = {0.0f};

    f32_T q0_diff = 0.0f;
    f32_T q1_diff = 0.0f;
    f32_T q2_diff = 0.0f;
    f32_T q3_diff = 0.0f;

    f32_T half_period = 0.0f;

    static uint32_T last_time = 0;
    uint32_T now_time = 0;

    /* 计算积分时间 */
    if(0 == last_time) /* 首次不积分 */
    {
        last_time = time;
        return;
    }
    now_time = time;

    /* TODO:复习半周期 */
    /* FIXME:此处有风险 */
    half_period = 0.5f * (now_time - last_time);
    last_time = now_time;

    /* 角度转弧度 */
    wx = math_angle2arc(wx);
    wy = math_angle2arc(wy);
    wz = math_angle2arc(wz);

    get_quaternion(q);

    /* 微分 */
    q0_diff =  -half_period * ( q[1] * wx + q[2] * wy + q[3] * wz);
    q1_diff =   half_period * ( q[0] * wx - q[3] * wy + q[2] * wz);
    q2_diff =   half_period * ( q[3] * wx + q[0] * wy - q[1] * wz);
    q3_diff =   half_period * (-q[2] * wx + q[1] * wy + q[0] * wz);

#if 0
    debug_log("q:%7.5f,%7.5f,%7.5f,%7.5f.\n", q[0], q[1], q[2], q[3]);
    debug_log("w:%5.2f,%5.2f,%5.2f.\n", wx,wy,wz);
    debug_log("half_period:%5.2f.\n", half_period);
    debug_log("diff:%7.5f,%7.5f,%7.5f,%7.5f.\n", q0_diff, q1_diff, q2_diff, q3_diff);
    debug_log("\n");
#endif

    /* 积分 */
    q[0] += q0_diff;
    q[1] += q1_diff;
    q[2] += q2_diff;
    q[3] += q3_diff;

    /* 归一化 */ 
    math_norm(q_norm, q, 4);

    set_quaternion(q_norm);

    return;
}

static void mpu9250_accel_fusion(uint32_T time, f32_T *accel)
{
;
    f32_T q[4] = {0.0f};

    f32_T theta_a = 0.0f;
    f32_T phi_a = 0.0f;

    f32_T theta_g = 0.0f;
    f32_T phi_g = 0.0f;
    f32_T theta_r = 0.0f;
    f32_T phi_r = 0.0f;
    f32_T euler[3] = {0.0f};

    /* TODO: 使用查表加速 */
    /* 水平直接姿态 */
    /* theta */
    theta_a = atan2(accel[1], accel[2]);
    /* phi */
    phi_a = atan2(accel[0], accel[2]);

    /* 获取四元数姿态 */
    get_quaternion(q);
    math_quaternion2euler(euler, q);
    /* 水平姿态 */
    theta_g = euler[0];
    phi_g = euler[1];

    /* 姿态融合 */
    theta_r = theta_g + (theta_a - theta_g) * ACCEL_THETA_RATE;
    phi_r = phi_g + (phi_a - phi_g) * ACCEL_PHI_RATE;

    euler[0] = theta_r;
    euler[1] = phi_r;
    /* 竖直方向留待compass校正 */
#if 0
    euler[2] = euler[2]; 
#endif

    /* 反推回四元数 */
    math_euler2quaternion(q, euler);
    set_quaternion(q); 
    
    s_accel[0] = accel[0];
    s_accel[1] = accel[1];
    s_accel[2] = accel[2];

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

static void ak8963_fusion(uint32_T time, f32_T *val, uint8_t st1, uint8_t st2)
{
    f32_T e[3] = {0.0f}; /* 指东针 */

    f32_T psi = 0.0f;
    f32_T psi_c = 0.0f;
    f32_T psi_g = 0.0f;
    f32_T euler[3] = {0.0f};
    f32_T q[4] = {0.0f};

    f32_T accel[3];
    f32_T compass[3];

    accel[0] = s_accel[0];
    accel[1] = s_accel[0];
    accel[2] = s_accel[0];

    compass[0] = val[0];
    compass[1] = val[1];
    compass[2] = val[2];

    /* 求指东针 */
    math_vector3_cross_product(e, accel, compass);

#if 0
    printf("a:%7.4f,%7.4f,%7.4f + ", accel[0], accel[1], accel[2]);
    printf("m:%7.4f,%7.4f,%7.4f => ", mag[0], mag[1], mag[2]);
    printf("e:%7.4f,%7.4f,%7.4f\n", e[0], e[1], e[2]);
#endif

    /* 直接姿态 */
    /* psi */
    psi_c = atan2(e[0], e[1]);

    /* 获取积分姿态竖直 */
    get_quaternion(q);
    math_quaternion2euler(euler, q);
    psi_g = euler[2];

    /* 姿态融合 */
    psi = psi_g + (psi_c - psi_g) * COMPASS_PSI_RATE;

    /* 水平方向留待accel校正 */
#if 0
    euler[0] = euler[0];
    euler[1] = euler[1];
#endif
    euler[2] = psi; 
    
    /* 关闭磁力计融合算法 */
#if 0
    /* 转换为四元数 */
    math_euler2quaternion(q, euler);
    set_quaternion(q);
#endif
}

