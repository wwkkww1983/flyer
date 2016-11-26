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
/* 主循环中输出间隔(ms) */
#define MAIN_LOOP_OUT_INTERVAL                              (5000)

/*********************************** 调试打印 **********************************/
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

/************************************* 协议 ************************************/
/* 下行协议帧长32Bytes = type 4Bytes + len 4Bytes + crc32 4Bytes + data 20Bytes */
#define COMM_DOWN_FRAME_BUF_SIZE                            (32U)
/* len 域数据 */
#define COMM_DOWN_FRAME_DATA_AND_CRC32_SIZE                 (COMM_DOWN_FRAME_BUF_SIZE - 8)

/*********************************** 硬件配置 **********************************/
/* 传感i2c速度 */
#define SI_RATE                                             (400000U)
/* MPU9250配置 */
#define MPU9250_ACCEL_FSR                                   (16U)
#define MPU9250_ACCEL_LPF                                   (10U)
#define MPU9250_GYRO_FSR                                    (2000U)
#define MPU9250_DMP_SAMPLE_RATE                             (100U)
#define MPU9250_SAMPLE_RATE                                 (1000U)
#define MPU9250_ORIENTATION                                 { 1, 0, 0, \
                                                              0, 1, 0, \
                                                              0, 0, 1}

/*********************************** 算法参数 **********************************/
#define CTRL_BASE_INIT_RATE                                 (0.0f)
#define CTRL_THETA_KP_INIT                                  (3.0f)
#define CTRL_THETA_KI_INIT                                  (0.0f)
#define CTRL_THETA_KD_INIT                                  (0.0f)
#define CTRL_THETA_EXPECT_INIT                              (0.0f)
#define CTRL_PHI_KP_INIT                                    (3.0f)
#define CTRL_PHI_KI_INIT                                    (0.0f)
#define CTRL_PHI_KD_INIT                                    (0.0f)
#define CTRL_PHI_EXPECT_INIT                                (0.0f)
#define CTRL_PSI_KP_INIT                                    (3.0f)
#define CTRL_PSI_KI_INIT                                    (0.0f)
#define CTRL_PSI_KD_INIT                                    (0.0f)
#define CTRL_PSI_EXPECT_INIT                                (0.0f)
/* 加计1阶滞后滤波比例参数 [0,1] 值增则平滑,灵敏度降 */
#define FILTER_ACCEL_1FACTOR_LAST_RATE                      (0.995f)
/* 加计滤波比例参数 [2,无穷) 值增则平滑,采样率降 */
#define FILTER_ACCEL_AVERAGE_NUMS                           (10)
/* 融合算法参数 */
#define FILTER_FUSION_ACCEL_THETA_RATE                      (0.5f)
#define FILTER_FUSION_ACCEL_PHI_RATE                        (0.5f)

/************************************* 常量 ************************************/
/* 弧度转角度 */
#define MATH_ARC2ANGLE_RATE                                 (180/MATH_PI)
/* 欧拉角 */
#define EULER_THETA                                         (0)
#define EULER_PHI                                           (1)
#define EULER_PSI                                           (2)
#define EULER_MAX                                           (3)
/* 欧拉角 */
#define QUAT_NUM                                            (4)
/* 坐标轴 */
#define AXES_NUM                                            (3)

/*********************************** 系统配置 **********************************/
/* 存储算法参数的FLASH起始地址和大小(与单片机硬件相关),最后16k */
#define FLASH_USER_START                                    ((uint32_T *)0x0801C000)
#define FLASH_USER_END                                      ((uint32_T *)0x08020000)
/* 每秒钟systick中断数 默认1ms */
#define TICK_PER_SECONDS                                    (1000U)

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/

#endif /* _CONFIG_H_ */

