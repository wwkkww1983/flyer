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
#include "lib_math.h"

/************************************ 宏定义 ***********************************/
/* _DEBUG_PC_
 * PC调试
 * */
/* #define _DEBUG_PC_ */

/************************************* 配置 ************************************/
/* 调试级别 */
/* 0: 不输出任何打印(仅输出协议帧) */
/* 1: 输出err_log */
/* 2: 输出debug_log */
/* 3: 输出trace_log */
#define DEBUG_LEVEL                                         (2)
/* console波特率 */
#define CONSOLE_BAUDRATE                                    (115200U)
/* esp8266波特率 */
#define ESP8266_BAUDRATE                                    (115200U)

/* uart 行缓冲的长度 64Bytes */
#define UART_LINE_BUF_SIZE                                  (64U)

/* 下行协议帧长32Bytes = type 4Bytes + len 4Bytes + crc32 4Bytes + data 20Bytes */
#define COMM_DOWN_FRAME_BUF_SIZE                            (32U)
/* len 域数据 */
#define COMM_DOWN_FRAME_DATA_AND_CRC32_SIZE                 (COMM_DOWN_FRAME_BUF_SIZE - 8)

/* Sensor i2c速度 */
#define SENSOR_I2C_RATE                                     (400000U)
#define MPU9250_ACCEL_FSR                                   (16U)
#define MPU9250_GYRO_FSR                                    (2000U)
#define MPU9250_DMP_SAMPLE_RATE                             (100U)
/* max rate 1000 */
#define MPU9250_SAMPLE_RATE                                 (1000U)
/* 配置MPU9250的方向 */
#define MPU9250_ORIENTATION                                 { 1, 0, 0, \
                                                              0, 1, 0, \
                                                              0, 0, 1}

/* 弧度转角度 */
#define MATH_ARC2ANGLE_RATE                                 (180/MATH_PI)

/* 加计滤波比例参数 [0,1] 值增大则平滑 */
#define FILTER_ACCEL_RATE                                   (0.999f)

/* 融合算法参数 */
#define FUSION_ACCEL_THETA_RATE                             (0.9f)
#define FUSION_ACCEL_PHI_RATE                               (0.9f)

/* 以下内容不可修改 */
/* 每秒钟systick中断数 默认1ms */
#define TICK_PER_SECONDS                                    (1000U)

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/

#endif /* _CONFIG_H_ */

