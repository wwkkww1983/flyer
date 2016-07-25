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
#include "console.h"
#include "esp8266.h"

/************************************ 宏定义 ***********************************/
/* 协议的详细定义见文档 */
#define COMM_FRAME_FLYER_CTRL_BIT               ((uint32_T)(1 << 30))
#define COMM_FRAME_SENSOR_DATA_BIT              ((uint32_T)(1 << 29))
#define COMM_FRAME_DIRECTION_BIT                ((uint32_T)(1 << 28))

#define COMM_FRAME_DMP_QUAT_BIT                 ((uint32_T)(1 << 1))
#define COMM_FRAME_TIME_BIT                     ((uint32_T)(1 << 0))

/* 上行帧最大帧长 */
#define COMM_FRAME_UP_FRAME_MAX_SIZE            ((uint32_T)(32))
/* 小于以下帧长不发送(可以用于控制上行帧数) */
#define COMM_FRAME_SENDED_MIN                   ((uint32_T)(12))
#define COMM_FRAME_FILLED_VAL                   ((uint8_T)(0xA5))

#define COMM_FRAME_INTERVAL_MAX                 ((uint32_T)(10000))


/*********************************** 实现函数 **********************************/
/*********************************** 类型定义 **********************************/
typedef struct{ 
    uint32_T type;
    uint32_T len;
    uint32_T interval;
    uint32_T crc;
}comm_frame_T;

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/ 
void comm_init(const drv_uart_T *comm_uart);
/* 通信任务 */
void comm_task(void);

#endif

