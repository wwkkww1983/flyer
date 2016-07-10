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
#include "misc.h"
#include "fusion.h"
#include "lib_math.h"
#include "sensor.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
static f32_T s_quaternion[MATH_QUAD] = {0.0f}; /* 四元数 q0 q1 q2 q3*/
//static data_T *s_ptr_data;
//static accel_T s_accel[ACCEL_DATA_PER_10MS];
//static gyro_T s_gyro[GYRO_DATA_PER_10MS];


/********************************** 函数声明区 *********************************/
static void set_quaternion(f32_T *q);
static void get_quaternion(f32_T *q);
static void mpu9250_gyro_fusion(uint32_T time, f32_T *gyro);
static void mpu9250_accel_fusion(uint32_T time, f32_T *accel);
static void ak8963_fusion(uint32_T time, f32_T *val, uint8_t st1, uint8_t st2);
//static void gyro_fusion(gyro_T *gyro);
//static void accel_fusion(accel_T *accel_v);
//static void compass_fusion(compass_T *compass_v, accel_T *accel_v);
//static void accel_filter(accel_T *accel, accel_T *accel_buf, int32_T buf_size);
/********************************** 变量实现区 *********************************/

/********************************** 函数实现区 *********************************/
/* 融合初始化 */
void fusion_init(void)
{
    f32_T euler[3] = {0.0f}; /* 欧拉角 全零 */
    f32_T q[4] = {0.0f}; /* 四元数 */

    math_euler2quaternion(q, euler);
    set_quaternion(q);

    console_printf("四元数初始值为:%7.4f, %7.4f, %7.4f, %7.4f\r\n", q[0], q[1], q[2], q[3]);
}

/* 10ms的数据作为单位计算 */
/* 可优化为1ms融合gyro accel, 10ms融合 compass */
void fusion(void)
{
//#define __DEBUG_RATE__
#ifdef __DEBUG_RATE__
    static misc_time_T time0 = {0, 0};
    static misc_time_T time1 = {0, 0};
    misc_time_T cost;
    
    /* 首次进入时为0 其他任意时刻都不为零(49天的溢出时间) */
    if((0 == time0.ms)
    && (0 == time0.clk))
    {
        get_now(&time0);
    }
#endif 
    sensor_data_T *data;

    /* step1: 确保数据获取已经获取 */
    if(!sensor_data_ready())
    {
        return;
    }

#ifdef __DEBUG_RATE__ 
    get_now(&time1);
    diff_clk(&cost, &time0, &time1);
    debug_log("等待数据耗时:%ums,%5.2fus.\r\n", cost.ms, 1.0f * cost.clk / 84); 
    
    get_now(&time0);
#endif

    /* step2: 获取已解析的数据指针 */ 
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

#if 0

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
#endif
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

inline static void get_quaternion(f32_T *q)
{ 
    q[0] = s_quaternion[0];
    q[1] = s_quaternion[1];
    q[2] = s_quaternion[2];
    q[3] = s_quaternion[3];
}

static void mpu9250_gyro_fusion(uint32_T time, f32_T *gyro)
{
    ;
}

static void mpu9250_accel_fusion(uint32_T time, f32_T *accel)
{
    ;
}

static void ak8963_fusion(uint32_T time, f32_T *val, uint8_t st1, uint8_t st2)
{
    ;
}

#if 0
/********************************** 变量实现区 *********************************/
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

#if 1
    static int32_T pp = 0;
    if(50 == pp)
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

#endif
