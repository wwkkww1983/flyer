/********************************************************************************
*
* 文件名  ： config.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150614
* 版本号  ： v1.0
* 文件描述： 配置
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _CONFIG_H_
#define _CONFIG_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"

/************************************ 宏定义 ***********************************/

/************************************ 调度 *************************************/
/****************************** 虚拟文件系统(VFS) ******************************/
/********************************** 控制台配置 *********************************/
/* 调试级别 */
/* 0: 输出err_log */
/* 1: 输出debug_log */
/* 2: 输出trace_log */
#define DEBUG_LEVEL                                         (2)

/* console 串口波特率 */
#define CONSOLE_BAUDRATE                                    (115200)

/* printf缓存 */
#define PRINTF_BUF_SIZE                                     (256)

/* imu AB面单面缓存大小单位 每个单位为sizeof(data_T) */
#define IMU_HALF_SIZE                                       (1000)

/* MPU9250 配置 */
//#define MPU9250_DMP_FIFO_RATE                             (40)
#define MPU9250_ACCEL_FSR                                   (2)
#define MPU9250_GYRO_FSR                                    (2000)
#define MPU9250_MAIN_SAMPLE_RATE                            (1000)
#define MPU9250_MAG_SAMPLE_RATE                             (100)

/* 以下内容不可修改 */
/* 每秒钟 systick中断数 默认1ms */
#define SLICE_PER_SECONDS                                   (1000)

/* 阻塞发送串口数据超时参数 */ 
#define UART_TIMEOUT_DIV                                    (1)

/* imu i2c 速率 400kHz */
#define IMU_RATE                                            (400000)

/* 1ms采样accel一次 */
#define IMU_ACCEL_READ_FREQ                                 (1)
/* 1ms采样gyro一次 */
#define IMU_GRRO_READ_FREQ                                  (1)
/* 10ms采样compass一次 */
#define IMU_COMPASS_READ_FREQ                               (10)
/* 10ms的accel数据量 */
#define ACCEL_DATA_PER_10MS                                 (10 * IMU_ACCEL_READ_FREQ)
/* 10ms的gyro数据量 */
#define GYRO_DATA_PER_10MS                                  (10 * IMU_GRRO_READ_FREQ)
/* 10ms的compass数据量 */
#define COMPASS_DATA_PER_10MS                               (10 / IMU_COMPASS_READ_FREQ)
/* 10ms的数据总量 */
#define DATA_PER_10MS                                       (ACCEL_DATA_PER_10MS + GYRO_DATA_PER_10MS + COMPASS_DATA_PER_10MS)

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/

#endif /* _CONFIG_H_ */

