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
#include "console.h"

/************************************ 宏定义 ***********************************/
/*********************************** 板级定义 **********************************/
#define MPU9250_DEV_ADDR                    (0xD0)
#define MPU9250_WHO_AM_I_REG_ADDR           (0x75)
#define MPU9250_WHO_AM_I_REG_VALUE          (0x71)
#define MPU9250_ACCEL_DATA_ADDR             (0x3B)
#define MPU9250_GYRO_DATA_ADDR              (0x43)
#define MPU9250_DATA_ADDR                   (MPU9250_ACCEL_DATA_ADDR)
#define MPU9250_DATA_LENGTH                 (14)

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
/* inv适配 */
#define mpu9250_log_i   debug_log
#define mpu9250_log_e   err_log
int mpu9250_read_buf(unsigned char dev_addr, unsigned char reg_addr,
        unsigned short buf_len, unsigned char *ptr_read_buf);
int mpu9250_write_buf(unsigned char dev_addr, unsigned char reg_addr, 
        unsigned short buf_len, const unsigned char *ptr_write_buf);
int mpu9250_get_ms(unsigned long *count);
void mpu9250_delay_ms(unsigned int ms);

/* 初始化 */
void mpu9250_init(void);

/* 测试 */
void mpu9250_test(void);

#endif /* _MPU9250_H_ */

