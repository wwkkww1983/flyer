/********************************************************************************
*
* 文件名  ： mpu9250.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150614
* 版本号  ： v1.0
* 文件描述： mpu9250驱动头文件
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _MPU9250_H_
#define _MPU9250_H_

/************************************ 头文件 ***********************************/
#include "config.h"
#include "stm32f4xx_hal.h"
#include "debug.h"
#include "misc.h"

/************************************ 宏定义 ***********************************/
/*********************************** 板级定义 **********************************/
#define MPU9250_DEV_ADDR                    (0xD0)
#define MPU9250_WHO_AM_I_REG_ADDR           (0x75)
#define MPU9250_WHO_AM_I_REG_VALUE          (0x71)
#define MPU9250_ACCEL_REG_ADDR              (0x3B)
#define MPU9250_ACCEL_LENGTH                (6)
#define MPU9250_GYRO_REG_ADDR               (0x43)
#define MPU9250_GYRO_LENGTH                 (6)
#define MPU9250_INT_STATUS_REG_ADDR         (0x3A)
#define MPU9250_ATG_REG_ADDR                (MPU9250_ACCEL_REG_ADDR)
#define MPU9250_ATG_LENGTH                  (14)

/*--------------------------------- 接口声明区 --------------------------------*/
typedef struct{ 
    f32_T data[AXES_NUM];   /* 数据 */
    uint16_T sens;          /* 灵敏度 */ 
}MPU9250_ACCEL_T;

typedef struct{ 
    f32_T data[AXES_NUM];   /* 数据 */
    f32_T sens;             /* 灵敏度 */ 
}MPU9250_GYRO_T;

#if 0
/* 联合节约空间 */
typedef union{
    uint8_T         type;   /* 数据类型 */
    MPU9250_ACCEL_T accel;  /* 加计 */
    MPU9250_GYRO_T  gyro;   /* 陀螺 */
    MPU9250_QUAT_T  quat;   /* DMP四元数 */
}MPU9250_T;
#endif

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
/* 初始化 */
void mpu9250_init(void);
/* 更新姿态 */
void mpu9250_update(void); 
/* 获取姿态(欧拉角) */
void mpu9250_get_euler(f32_T *euler);
/* 获取姿态(欧拉角) */
bool_T mpu9250_euler_updated(void);
/* 清理上次姿态(欧拉角已经被控制模块使用) */
void mpu9250_clear_euler(void);

/* 获取dmp四元数 */
void mpu9250_get_dmp_quat(f32_T *dmp_quat);
/* 获取加计数据 */
void mpu9250_get_accel(f32_T *accel);
/* 获取采样最大间隔 */
void mpu9250_get_interval_max(misc_time_T *interval);

#endif /* _MPU9250_H_ */

