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
/* 三个被调量:俯仰角 横滚角 偏航角 */
static PID_T s_pid[EULER_MAX];
/* 电机油门 */
static CTRL_T s_ctrl[PWM_MAX];

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
void ctrl_init(void)
{
    int32_T i = 0;

    /* pid:俯仰角 横滚角 偏航角 依次初始化 */
    /* 俯仰角初始化 */
    s_pid[EULER_THETA].kp = CTRL_THETA_KP_INIT;
    s_pid[EULER_THETA].ki = CTRL_THETA_KI_INIT;
    s_pid[EULER_THETA].kd = CTRL_THETA_KD_INIT;
    s_pid[EULER_THETA].expect = CTRL_THETA_EXPECT_INIT;
    s_pid[EULER_THETA].acc = 0;

    /* 横滚角初始化 */
    s_pid[EULER_PHI].kp = CTRL_PHI_KP_INIT;
    s_pid[EULER_PHI].ki = CTRL_PHI_KI_INIT;
    s_pid[EULER_PHI].kd = CTRL_PHI_KD_INIT;
    s_pid[EULER_PHI].expect = CTRL_PHI_EXPECT_INIT;
    s_pid[EULER_PHI].acc = 0;

    /* 偏航角初始化 */
    s_pid[EULER_PSI].kp = CTRL_PSI_KP_INIT;
    s_pid[EULER_PSI].ki = CTRL_PSI_KI_INIT;
    s_pid[EULER_PSI].kd = CTRL_PSI_KD_INIT;
    s_pid[EULER_PSI].expect = CTRL_PSI_EXPECT_INIT;
    s_pid[EULER_PSI].acc = 0; 
    
    /* 初始化油门为0 */
    for(i = 0; i < EULER_MAX; i++)
    {
        s_ctrl[i].base = (int32_T)(pwm_get_period() * CTRL_BASE_INIT_RATE);
        s_ctrl[i].adj  = 0;
    }
}

void ctrl_update(void)
{
    f32_T euler_measured[EULER_MAX] = {0.0f};
    f32_T out[EULER_MAX] =  {0.0f};

    int32_T i = 0;
    int32_T pwm_val_max = 0;
    int32_T pwm_val_half_max = 0;
    int32_T adj_max = 0;
    int32_T pwm = 0;

    /* step1: 获取姿态 */
    /* 无新姿态 故无需更新pwm */
    if(!mpu9250_euler_updated())
    {
        return;
    }
    mpu9250_get_euler(euler_measured); /* 获取且标记 */
    mpu9250_clear_euler(); /* 避免多次控制 */

    /* step2: pid算法计算校正值 */
    for(i = 0; i < EULER_MAX; i++)
    {
        pid_update(&s_pid[i], euler_measured[i]); 
    } 
    /* 获取pid输出 */
    ctrl_get_pid_out(out);

    /* step3: 计算纠偏值 */
    /* 俯仰角平衡 */
    s_ctrl[PWM_NE1].adj += out[EULER_THETA];
    s_ctrl[PWM_SE2].adj += out[EULER_THETA];
    s_ctrl[PWM_SW3].adj -= out[EULER_THETA];
    s_ctrl[PWM_NW4].adj -= out[EULER_THETA];

    /* 横滚角平衡 */
    s_ctrl[PWM_SE2].adj -= out[EULER_PHI];
    s_ctrl[PWM_SW3].adj -= out[EULER_PHI];
    s_ctrl[PWM_NE1].adj += out[EULER_PHI];
    s_ctrl[PWM_NW4].adj += out[EULER_PHI];

    /* TODO:处理偏航 */
    /* 限幅 */
    pwm_val_max = pwm_get_period();
    pwm_val_half_max = pwm_get_period() / 2;
    for(i = 0; i < PWM_MAX; i++)
    { 
        /* step1: 计算adj范围[-adj_max, adj_max] */
        if(s_ctrl[i].base < pwm_val_half_max) /* 油门较小 [0, base] */
        {
            adj_max = s_ctrl[i].base;
        }
        else if(s_ctrl[i].base > pwm_val_half_max) /* 油门较大 [base, max] */
        {
            adj_max = pwm_val_max - s_ctrl[i].base;
        }
        else /* [0, max/2] */
        {
            adj_max = pwm_val_half_max;
        }

        /* step2: 避免adj太过偏离(响应速度慢) */
        if(s_ctrl[i].adj > adj_max)
        {
            s_ctrl[i].adj = adj_max;
        }
        else if (s_ctrl[i].adj < -adj_max)
        {
            s_ctrl[i].adj = -adj_max;
        }
        else /* adj 在 [-adj_max, adj_max] 内不处理 */
        { ; }
    }

    /* step4: 控制油门 */
    for(i = 0; i < PWM_MAX; i++)
    {
        pwm = (int32_T)(s_ctrl[i].base + s_ctrl[i].adj);
        pwm_set((PWM_NAME)i, pwm);
    }

    /* TODO: 不翻跟头后删除 开始 */
    if((euler_measured[0] < -60.0f)
     ||(euler_measured[0] >  60.0f))
    { 
        ctrl_motor_off();
    }

    if((euler_measured[1] < -60.0f)
     ||(euler_measured[1] >  60.0f))
    { 
        ctrl_motor_off();
    }
    /* TODO: 不翻跟头后删除 结束 */

}

void ctrl_set_pid(int32_T euler_index, const PID_T *pid)
{
    /* 参数检查 */
    if((NULL == pid)
    || !((EULER_THETA == euler_index) || (EULER_PHI == euler_index) || (EULER_PSI == euler_index)))
    { 
        ERR_STR("参数错误");
    } 

    s_pid[euler_index].kp = pid->kp;
    s_pid[euler_index].ki = pid->ki;
    s_pid[euler_index].kd = pid->kd; 
}

void ctrl_set_expect(int32_T euler_index, f32_T expect)
{
    /* 参数检查 */
    if(!((EULER_THETA == euler_index) || (EULER_PHI == euler_index) || (EULER_PSI == euler_index)))
    { 
        ERR_STR("参数错误");
    } 

    s_pid[euler_index].expect = expect;
}

void ctrl_get_pid_out(f32_T *out)
{
    int32_T i = 0;

    /* 参数检查 */
    if(NULL == out)
    {
        ERR_STR("参数错误");
    }

    for(i = 0; i < EULER_MAX; i++)
    {
        out[i] = s_pid[i].out;
    } 
}

void ctrl_motor_off(void)
{ 
    int32_T val[PWM_MAX] = {0,0,0,0};

    pwm_off();
    ctrl_set_acceleralor(val);
}

/* 设置base值 */
void ctrl_set_acceleralor(const int32_T *val)
{
    int32_T i = 0;
    int32_T val_temp = 0;
    int32_T val_max = 0;

    if(NULL == val)
    {
        ERR_STR("参数错误");
    }

    val_max = pwm_get_period();

    for(i = 0; i < PWM_MAX; i++)
    {
        /* 限幅 */
        val_temp = val[i];
        if(val_temp > val_max)
        {
            val_temp = val_max;
        }
        if(val_temp < 0)
        {
            val_temp = 0;
        }

        s_ctrl[i].base = val_temp;
    }
}

/* 获取实时值 */
void ctrl_get_acceleralor(int32_T *val, int32_T *val_max)
{
    int32_T i = 0;
    if((NULL == val)
    || (NULL == val_max))
    {
        ERR_STR("参数错误");
    }

    for(i = 0; i < PWM_MAX; i++) 
    {
        val[i] = (int32_T)(s_ctrl[i].base + s_ctrl[i].adj);
    }

    *val_max = pwm_get_period();
}

