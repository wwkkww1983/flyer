/******************************************************************************
 *
 * 文件名  ： ctrl.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20150825 
 * 版本号  ： v1.0
 * 文件描述： 飞行控制模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "ctrl.h"
#include "pwm.h"
#include "mpu9250.h"
#include "pid.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* 三个控制量:俯仰角 横滚角 偏航角 */
static PID_T s_pid[CTRL_EULER_MAX];

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
void ctrl_init(void)
{
    /* 俯仰角 横滚角 偏航角 依次初始化 */
    /* 俯仰角初始化 */
    s_pid[CTRL_THETA].kp = CTRL_THETA_KP_INIT;
    s_pid[CTRL_THETA].ki = CTRL_THETA_KI_INIT;
    s_pid[CTRL_THETA].kd = CTRL_THETA_KD_INIT;
    s_pid[CTRL_THETA].expect = CTRL_THETA_EXPECT_INIT;
    s_pid[CTRL_THETA].acc = 0;

    /* 横滚角初始化 */
    s_pid[CTRL_PHI].kp = CTRL_PHI_KP_INIT;
    s_pid[CTRL_PHI].ki = CTRL_PHI_KI_INIT;
    s_pid[CTRL_PHI].kd = CTRL_PHI_KD_INIT;
    s_pid[CTRL_PHI].expect = CTRL_PHI_EXPECT_INIT;
    s_pid[CTRL_THETA].acc = 0;

    /* 偏航角初始化 */
    s_pid[CTRL_PSI].kp = CTRL_PSI_KP_INIT;
    s_pid[CTRL_PSI].ki = CTRL_PSI_KI_INIT;
    s_pid[CTRL_PSI].kd = CTRL_PSI_KD_INIT;
    s_pid[CTRL_PSI].expect = CTRL_PSI_EXPECT_INIT;
    s_pid[CTRL_THETA].acc = 0;
}

void ctrl_update(void)
{
    f32_T quat_measured[4] = {0.0f}; 
    f32_T euler_measured[CTRL_EULER_MAX] = {0.0f};
    f32_T out[CTRL_EULER_MAX] =  {0.0f};

    static f32_T pwm_val_f[4] = {0.0f}; /* 浮点用于记忆out对应的pwm_val的浮点值(确保小out值时精度不丢失) */
    int32_T pwm_val[4] = {0}; /* 依次为前右后左 */
    int32_T i = 0;

    /* 无新四元数 故无需更新pwm */
    if(!mpu9250_quat_arrived())
    {
        return;
    }
    mpu9250_get_quat_with_clear(quat_measured); /* 获取且标记 */
    math_quaternion2euler(euler_measured, quat_measured);

    /* pid算法计算校正值 */
    for(i = 0; i < CTRL_EULER_MAX; i++)
    {
        pid_update(&s_pid[i], euler_measured[i]); 
    } 
    
    /* 获取pid输出 */
    ctrl_get_pid_out(out);

    /* theta */
    pwm_val_f[PWM_FRONT] += out[CTRL_THETA] / 2.0f;
    pwm_val_f[PWM_BACK]  -= out[CTRL_THETA] / 2.0f;

    /* 横滚 偏航 都处理 */
#if 0
    /* phi */
    out[1];

    /* psi */
    out[2]; 
#endif

    for(i = 0; i<4; i++)
    {
        pwm_val[i] = (int32_T)(pwm_val_f[i]);
    }

    pwm_update(pwm_val);
}

void ctrl_set_pid(const PID_T *pid)
{
    int32_T i = 0;
    /* 参数检查 */
    if(NULL == pid)
    {
        while(1);
    }

    for(i = 0; i < CTRL_EULER_MAX; i++)
    {
        s_pid[i].kp = pid[i].kp;
        s_pid[i].ki = pid[i].ki;
        s_pid[i].kd = pid[i].kd;

        s_pid[i].expect = pid[i].expect;
    } 
}

void ctrl_get_pid_out(f32_T *out)
{

    int32_T i = 0;

    /* 参数检查 */
    if(NULL == out)
    {
        while(1);
    }

    for(i = 0; i < CTRL_EULER_MAX; i++)
    {
        out[i] = s_pid[i].out;
    } 
}

