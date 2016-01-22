/******************************************************************************
 *
 * 文件名  ： fusion
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160116 
 * 版本号  ： v1.0
 * 文件描述： 融合算法
 * 版权说明： Copyright (c) 2000-2020 GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/


/*---------------------------------- 预处理区 ---------------------------------*/
/* 消除中文打印警告 */
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include <math.h>

#include "fusion.h"
#include "lib_math.h"
#include "imu.h"
#include "data.h"
#include "console.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
static f32_T s_quaternion[MATH_QUAD] = {0.0f}; /* 四元数 q0 q1 q2 q3*/

/********************************** 函数声明区 *********************************/
static void lock(void);
static void unlock(void);

static void set_quaternion(f32_T *q);
static void get_quaternion(f32_T *q);

static void gyro_fusion(gyro_T *gyro);
static void accel_fusion(accel_T *accel);
static void compass_fusion(compass_T *compass, accel_T *accel);
static void accel_filter(accel_T *accel, accel_T *accel_buf, int32_T buf_size);

/********************************** 变量实现区 *********************************/
static data_T *s_ptr_data;
static accel_T s_accel[ACCEL_DATA_PER_10MS];
static gyro_T s_gyro[GYRO_DATA_PER_10MS];

/********************************** 函数实现区 *********************************/
/* 10ms的数据作为单位计算 */
/* 可优化为1ms融合gyro accel, 10ms融合 compass */
void fusion(void)
{
//#define __DEBUG_RATE__
#ifdef __DEBUG_RATE__
    uint32_T start = 0;
    uint32_T end = 0;
    uint32_T cost = 0;
    start = HAL_GetTick();
#endif

    int32_T i = 0;
    int32_T accel_i = 0;
    int32_T gyro_i = 0;
    int32_T compass_i = 0;

    accel_T accel;
    compass_T compass;

    data_type_T type = ERR_E;
    uint32_T valid_buf_size = 0;

    /* step1: 获取数据 */
    valid_buf_size = imu_read_buf(&s_ptr_data, DATA_PER_10MS);
    if(0 == valid_buf_size) /* 无有效数据 */
    {
        //debug_log("融合速率较快,可以正常工作\r\n");
        return;
    }

    /* step2: 解析数据 */
    for(i = 0; i < valid_buf_size; i++)
    {
        type = s_ptr_data[i].type; 

        switch(type)
        {
            case accel_E:
                {
                    assert_param(accel_i < ACCEL_DATA_PER_10MS);
                    parse_accel(&s_accel[accel_i], &s_ptr_data[i]);
                    accel_i++;
                    break;
                }
            case gyro_E:
                {
                    assert_param(gyro_i < GYRO_DATA_PER_10MS);
                    parse_gyro(&s_gyro[gyro_i], &s_ptr_data[i]);
                    gyro_i++;
                    break;
                }
            case compass_E:
                {
                    assert_param(compass_i < COMPASS_DATA_PER_10MS); /* 10ms 仅采样一次 */
                    parse_compass(&compass, &s_ptr_data[i]);
                    compass_i++;
                    break;
                }
            default:
                {
                    ERR_STR("fusion 获取的数据类型错误.\r\n");
                    while(1);
                }
        }
    }

#ifdef __DEBUG_RATE__
    end = HAL_GetTick();
    cost = end - start;
    debug_log("融合准备耗时: %ums.\r\n", cost);

    start = HAL_GetTick();
#endif
    /* step3: gyro融合 */
    for(i = 0; i < gyro_i; i++)
    {
        gyro_fusion(&s_gyro[i]);
    }

#ifdef __DEBUG_RATE__
    end = HAL_GetTick();
    cost = end - start;
    debug_log("gyro_i融合%d次耗时: %ums.\r\n", gyro_i, cost);
    
    start = HAL_GetTick();
#endif

    /* step4: accel融合 速度慢 所以10ms融合一次 */
    accel_filter(&accel, s_accel, accel_i);
#ifdef __DEBUG_RATE__
    end = HAL_GetTick();
    cost = end - start;
    debug_log("accel滤波1次耗时: %ums.\r\n", cost);
    start = HAL_GetTick();
#endif
    accel_fusion(&accel);
#ifdef __DEBUG_RATE__
    end = HAL_GetTick();
    cost = end - start;
    debug_log("accel融合1次耗时: %ums.\r\n", cost);
    
    start = HAL_GetTick();
#endif
    /* step5: compass融合 速度慢 所以10ms融合一次 */
    compass_fusion(&compass, &accel);
#ifdef __DEBUG_RATE__
    end = HAL_GetTick();
    cost = end - start;
    debug_log("compass融合1次耗时: %ums.\r\n", cost);
#endif

#ifdef __DEBUG_RATE__
    /* stepx: 调试打印 */
    static uint32_T s_tick_last = 0;
    uint32_T s_tick_now = 0;
    f32_T q[MATH_QUAD];
    f32_T e[MATH_THREE];
    s_tick_now = HAL_GetTick();
    if((s_tick_now - s_tick_last) > 500) /* 500 ms 输出一次 */
    {
        get_quaternion(q);
        math_quaternion2euler(e, q);
        debug_log("%ld,%.4f, %.4f, %.4f <==> %.4f,%.4f,%.4f,%.4f\r\n", s_tick_now, e[0], e[1], e[2], q[0], q[1], q[2], q[3]);
        for(i = 0; i < accel_i; i++)
        {
            debug_log("accel     :%d,% 5ld,%10.4f,%10.4f,%10.4f\r\n", i, s_accel[i].time, s_accel[i].data[0], s_accel[i].data[1], s_accel[i].data[2]);
        }

        for(i = 0; i < gyro_i; i++)
        {
            debug_log("gyro      :%d,% 5ld,%10.4f,%10.4f,%10.4f\r\n", i, s_gyro[i].time, s_gyro[i].data[0], s_gyro[i].data[1], s_gyro[i].data[2]);
        } 
        
        debug_log("compass   :%d,% 5ld,%10.4f,%10.4f,%10.4f\r\n", i, compass.time, compass.data[0], compass.data[1], compass.data[2]);

        debug_log("\r\n");
        s_tick_last = s_tick_now;
    }
#endif
}

void fusion_init(void)
{
    f32_T euler[MATH_THREE] = {0.0f}; /* 欧拉角 全零 */
    f32_T q[MATH_QUAD] = {0.0f}; /* 四元数 */

    math_euler2quaternion(q, euler);
    set_quaternion(q);

    debug_log("初始化姿态四元数为:\r\n");
    debug_log("%.4f,%.4f,%.4f,%.4f\r\n", q[0], q[1], q[2], q[3]); 
}

void fusion_test_10ms_time(void)
{
    uint32_T start = 0;
    uint32_T end = 0;
    uint32_T cost = 0;

    /* 测试imu缓冲单面性能 */
    start = HAL_GetTick(); 
    /* 等待缓冲好第一面缓存 */
    imu_init_done_wait();
    end = HAL_GetTick(); 
    cost = end - start;
    debug_log("imu 缓冲一面缓存需要%ums(读取%.3fkBytes)\r\n", cost, 1.0f * sizeof(data_T) * IMU_HALF_SIZE / 1024);

    /* 测试融合算法性能 */
    start = HAL_GetTick(); 
    fusion();
    end = HAL_GetTick();
    cost = end - start;

    debug_log("融合1次耗时: %ums.\r\n", cost);

    if(cost > (10 / 3) ) /* stm32f401性能比stm32f429 低3倍 */
    {
        imu_stop(); /* 避免imu持续度调用串口输出报错 */
        debug_log("融合算法性能不达标(移除数据打印110ms > 3ms).\r\n"); 
        while(1);
    }
}

static void gyro_fusion(gyro_T *gyro)
{
    f32_T wx = gyro->data[0];
    f32_T wy = gyro->data[1];
    f32_T wz = gyro->data[2];

    f32_T q[MATH_QUAD] = {0.0f};
    f32_T q_norm[MATH_QUAD] = {0.0f};

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
        last_time = gyro->time;
        return;
    }
    now_time = gyro->time;

    half_period = HALF_PERIOD_CONST * (now_time - last_time);
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

static void accel_fusion(accel_T *accel_v)
{
    f32_T accel[MATH_THREE];
    f32_T q[MATH_QUAD] = {0.0f};

    f32_T theta_a = 0.0f;
    f32_T phi_a = 0.0f;

    f32_T theta_g = 0.0f;
    f32_T phi_g = 0.0f;
    f32_T theta_r = 0.0f;
    f32_T phi_r = 0.0f;
    f32_T euler[MATH_THREE] = {0.0f};

    accel[0] = accel_v->data[0];
    accel[1] = accel_v->data[1];
    accel[2] = accel_v->data[2];

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

    static int32_T pp = 0;
    if(50 == pp)
    {
        debug_log("%.4f,%.4f,%.4f\r\n", euler[0], euler[1], euler[2]);
        debug_log("%.4f,%.4f,%.4f,%.4f\r\n", q[0], q[1], q[2], q[3]);
        pp = 0;
    }
    pp++;

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

static void compass_fusion(compass_T *compass_v, accel_T *accel_v)
{
    float e[MATH_THREE] = {0.0f}; /* 指东针 */

    float psi = 0.0f;
    float psi_c = 0.0f;
    float psi_g = 0.0f;
    float euler[MATH_THREE] = {0.0f};
    float q[MATH_QUAD] = {0.0f};

    float accel[MATH_THREE];
    float compass[MATH_THREE];

    accel[0] = accel_v->data[0];
    accel[1] = accel_v->data[1];
    accel[2] = accel_v->data[2];

    compass[0] = compass_v->data[0];
    compass[1] = compass_v->data[1];
    compass[2] = compass_v->data[2];

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
    
    /* 转换为四元数 */
    math_euler2quaternion(q, euler);
    set_quaternion(q);
}

static void accel_filter(accel_T *accel, accel_T *accel_buf, int32_T buf_size)
{
    /* FIXME: 优化滤波 */
    /* 目前仅仅求均值 */
    int32_T i = 0;
    f32_T data[MATH_THREE] = {0.0f};

    for(i = 0; i < buf_size; i++)
    { 
        data[0] += accel_buf[i].data[0];
        data[1] += accel_buf[i].data[1];
        data[2] += accel_buf[i].data[2];
    } 
    
    data[0] /= buf_size;
    data[1] /= buf_size;
    data[2] /= buf_size;

    accel->data[0] = data[0];
    accel->data[1] = data[1];
    accel->data[2] = data[2];

    /* 每次间隔1ms 取中间值 */
    accel->time = accel_buf[0].time + buf_size / 2;
}

/* FIXME:考虑是否有并发问题 */
static void get_quaternion(f32_T *q)
{ 
    lock();

    q[0] = s_quaternion[0];
    q[1] = s_quaternion[1];
    q[2] = s_quaternion[2];
    q[3] = s_quaternion[3];

    unlock();
}

static void set_quaternion(f32_T *q)
{
    lock();

    s_quaternion[0] = q[0];
    s_quaternion[1] = q[1];
    s_quaternion[2] = q[2];
    s_quaternion[3] = q[3];

    unlock();
}

inline static void lock(void)
{}

inline static void unlock(void)
{}

