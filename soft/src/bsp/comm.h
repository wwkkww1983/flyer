/********************************************************************************
*
* 文件名  ： comm.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150721
* 版本号  ： v1.0
* 文件描述： 与上位机交互模块
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _COMM_H_
#define _COMM_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"
#include "uart.h"

/************************************ 宏定义 ***********************************/
/* 协议的详细定义见文档 */
#define COMM_FRAME_DIRECTION_BIT                ((uint32_T)(0x80000000))
#define COMM_FRAME_FLYER_CTRL_BIT               ((uint32_T)(0x40000000))
#define COMM_FRAME_SENSOR_DATA_BIT              ((uint32_T)(0x20000000))
#define COMM_FRAME_PID_CTRL_BIT                 ((uint32_T)(0x10000000))

#define COMM_FRAME_PID_DATA_BIT                 ((uint32_T)(0x00000100))
#define COMM_FRAME_EULER_DATA_BIT               ((uint32_T)(0x00000080))
#define COMM_FRAME_ACCELERATOR_DATA_BIT         ((uint32_T)(0x00000040))
#define COMM_FRAME_PRESS_DATA_BIT               ((uint32_T)(0x00000020))
#define COMM_FRAME_COMPASS_DATA_BIT             ((uint32_T)(0x00000010))
#define COMM_FRAME_GYRO_DATA_BIT                ((uint32_T)(0x00000008))
#define COMM_FRAME_ACCEL_DATA_BIT               ((uint32_T)(0x00000004))
#define COMM_FRAME_DMP_QUAT_BIT                 ((uint32_T)(0x00000002))
#define COMM_FRAME_PRINTF_BIT                   ((uint32_T)(0x00000001))

#define COMM_FRAME_TIME_DATA_SIZE               (4)
#define COMM_FRAME_DMP_QUAT_DATA_SIZE           (16)
#define COMM_FRAME_ACCELERATOR_QUAT_DATA_SIZE   (20)
#define COMM_FRAME_EULER_DATA_SIZE              (12)
#define COMM_FRAME_PID_DATA_SIZE                (12)

/* type+len+time+crc 长度 16Bytes */
#define COMM_FRAME_TYPE_TIME_LEN_CRC_SIZE       ((uint32_T)(16))
/* 采样上行帧最大帧长
 * FIXME: 有新的采样帧需要修改
 * */
#define COMM_FRAME_CAPTURE_FRAME_MAX_SIZE       ((uint32_T)(100))

/* 小于以下帧长不发送(可以用于控制上行帧数) */
#define COMM_FRAME_SENDED_MIN                   (COMM_FRAME_TYPE_TIME_LEN_CRC_SIZE)
#define COMM_FRAME_FILLED_VAL                   ((uint8_T)(0xA5))
#define COMM_FRAME_INTERVAL_MAX                 ((uint32_T)(10000))

/* 握手信号的发送间隔 */
#define COMM_SEND_HELLO_DELAY                   (100U)

/*********************************** 实现函数 **********************************/
/*********************************** 类型定义 **********************************/
typedef struct{ 
    uint32_T type;
    uint32_T len;
    uint32_T data;
    uint32_T crc;
}comm_frame_T;

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/ 
void comm_init(const drv_uart_T *comm_uart);
/* 通信任务 */
void comm_update(void); 

void comm_frame_printf_make(uint32_T *frame_len, uint8_T *frame_buf, uint32_T n);
#endif

