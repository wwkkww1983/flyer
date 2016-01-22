/******************************************************************************
 *
 * 文件名  ： data.h
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160116 
 * 版本号  ： v1.0
 * 文件描述： 数据处理 接口
 * 版权说明： Copyright (c) 2000-2020   烽火通信科技股份有限公司
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _DATA_H_
#define _DATA_H_

/************************************ 头文件 ***********************************/
#include "config.h"

/************************************ 宏定义 ***********************************/
/* gyro & accel 6 bytes, compass 8bytes, 按照最大的来 */
#define GYRO_BUF_SIZE               (6)
#define ACCEL_BUF_SIZE              (6)
#define COMPASS_BUF_SIZE            (8)
#define DATA_TYPE_BUF_SIZE          (COMPASS_BUF_SIZE)

/*********************************** 类型定义 **********************************/
/* 可以或运算 */
typedef enum{
    accel_E = 0x01,
    gyro_E  = 0x02,
    compass_E = 0x04,
    ERR_E = 0xff
}data_type_T;

typedef struct{
    unsigned long time;
    float data[3];
}accel_T;

typedef struct{
    unsigned long time;
    float data[3];
}gyro_T;

typedef struct{
    unsigned long time;
    float data[3];
}compass_T;

typedef struct{
    data_type_T type; /* 类型 */
    uint32_T time; /* 时间 */
    uint8_T buf[COMPASS_BUF_SIZE]; /* 缓存 */
}data_T;

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void data_config(void); 
void parse_accel(accel_T *accel, const data_T *data);
void parse_gyro(gyro_T *gyro, const data_T *data);
void parse_compass(compass_T *compass, const data_T *data);

#endif

