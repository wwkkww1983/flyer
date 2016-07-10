/********************************************************************************
*
* 文件名  ： sensor.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160112
* 版本号  ： v1.0
* 文件描述： sensor共有接口
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _SENSOR_H_
#define _SENSOR_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"

/************************************ 宏定义 ***********************************/

/*********************************** 类型定义 **********************************/
typedef enum{
    mpu9250_E = 0x01,
    ak8963_E  = 0x02,
    ERR_E = 0xff
}sensor_data_type_T;

typedef struct{
    f32_T accel[3];
    f32_T gyro[3];
    f32_T temp;
}mpu9250_val_T;

typedef struct{
    f32_T val[3];
    uint8_T st1;
    uint8_T st2;
}ak8963_val_T;

typedef union{
    mpu9250_val_T mpu9250;
    ak8963_val_T ak8963;
}sensor_val_T;

typedef struct{
    sensor_data_type_T type;
    uint32_T time;
    sensor_val_T val;
}sensor_data_T; /* 解析后的数据 */

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void sensor_init(void);
void sensor_test(void);
void sensor_read(void);
void sensor_get_sens(uint16_T *accel_sens, f32_T *gyro_sens, int16_T *mag_sens);

bool_T sensor_data_ready(void);
void sensor_get_data(sensor_data_T **data);

#endif

