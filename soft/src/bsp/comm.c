/******************************************************************************
 *
 * 文件名  ： comm.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160721 
 * 版本号  ： 1.0
 * 文件描述： 交互模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "config.h"
#include "typedef.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stm32f4xx_hal.h>

#include "board.h"
#include "uart.h"
#include "esp8266.h"
#include "console.h"
#include "comm.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
const drv_uart_T *s_comm_uart = NULL;
static CRC_HandleTypeDef s_crc;

static uint32_T s_send_interval = 0;
static bool_T s_send_time_flag = FALSE;
static bool_T s_send_dmp_quat_flag = FALSE;

/********************************** 函数声明区 *********************************/
static void parse(const uint8_T *frame);
static void send_capture_data(void);
inline static bool_T is_down_frame(uint32_T type);
inline static bool_T bit_compare(uint32_T type, uint32_T bit_mask);
inline static bool_T is_flyer_crtl_frame(uint32_T type);
inline static bool_T is_sensor_data_frame(uint32_T type);
inline static bool_T is_dmp_quat_needded(uint32_T type);
inline static bool_T is_time_needded(uint32_T type);

/********************************** 函数实现区 *********************************/
void comm_init(const drv_uart_T *comm_uart)
{
    /* 配置 协议走的通道 console or esp8266 */
    if(NULL == comm_uart)
    {
        while(1);
    }
    s_comm_uart =  comm_uart;

    /* 配置硬件CRC32 */
    s_crc.Instance = CRC32;
    CRC_CLK_ENABLE();
    if(HAL_OK != HAL_CRC_Init(&s_crc))
    {
        while(1);
    }
}

/* 通信交互任务 */
void comm_task(void)
{ 
    static bool_T first_run = TRUE; 
    static uint8_T frame_buf[COMM_DOWN_FRAME_BUF_SIZE] = {0};

    send_capture_data();

    if(first_run) /* 首次运行启动串口接收 */
    {
        first_run = FALSE; 
        
        /* 启动下行帧接收 */
        uart_recv_bytes((drv_uart_T *)s_comm_uart, frame_buf, COMM_DOWN_FRAME_BUF_SIZE);
    }
    else /* 非首次运行 等待帧 & 解析帧 & 启动串口接收 */
    { 
        /* 等待上一帧接收完成 */
        if(uart_frame_ready(s_comm_uart))
        {
            /* 帧未到达 退出(等待下一轮) */
            return;
        }
        else
        { 
            /* 解析(包含处理) */
            parse(frame_buf); 
            
            /* 启动下行帧接收 */
            uart_recv_bytes((drv_uart_T *)s_comm_uart, frame_buf, COMM_DOWN_FRAME_BUF_SIZE);
        }
    }
}

static void parse(const uint8_T *buf)
{
    comm_frame_T frame = {0};
    uint32_T crc32_calculated = 0;

    frame.type  = buf[0] << 24;
    frame.type |= buf[1] << 16;
    frame.type |= buf[2] << 8;
    frame.type |= buf[3];

    frame.len   = buf[4] << 24;
    frame.len  |= buf[5] << 16;
    frame.len  |= buf[6] << 8;
    frame.len  |= buf[7];

    frame.interval  = buf[8] << 24;
    frame.interval |= buf[9] << 16;
    frame.interval |= buf[10] << 8;
    frame.interval |= buf[11];

    frame.crc   = buf[12] << 24;
    frame.crc  |= buf[13] << 16;
    frame.crc  |= buf[14] << 8;
    frame.crc  |= buf[15];

    /* 长度检查 固定长度 16Bytes */
    if(COMM_DOWN_FRAME_BUF_SIZE != frame.len) /* 错误帧 不处理 */
    {
        return;
    }

    /* CRC检查 */ 
    crc32_calculated = HAL_CRC_Calculate(&s_crc, (uint32_T *)buf, COMM_DOWN_FRAME_BUF_SIZE / sizeof(uint32_T) - 1); 
    if(crc32_calculated != frame.crc) /* 校验失败*/
    {
        return;
    }

    /* 类型检查 */ 
    if(!is_down_frame(frame.type)) /* 错 收到上行帧 */
    {
        return;
    }
   
    /* 未实现飞控帧 */
    if(is_flyer_crtl_frame(frame.type))
    {
        while(1);
    }

    /* 传感数据请求帧 */
    /* 目前仅支持time+dmp_quat采样 */
    if( (is_sensor_data_frame(frame.type))
     && (is_dmp_quat_needded(frame.type))
     && (is_time_needded(frame.type)))
    { 
        /* 限制时间间隔(无符号32位 必然大于0不用比下界) */
        if(frame.interval > COMM_FRAME_INTERVAL_MAX)
        {
            frame.interval = COMM_FRAME_INTERVAL_MAX;
        }

        /* 启动time + dmp quat发送 */
        s_send_time_flag = TRUE;
        s_send_dmp_quat_flag = TRUE;
        s_send_interval = frame.interval;
    }
    else /* 暂时不支持其他采样 */
    {
        return;
    }
}

/* main.c中定义 */
void get_quat(f32_T *q);
/* 发送采样数据 */
static void send_capture_data(void)
{
    uint32_T now_ms = 0;
    uint8_T frame_buf[COMM_FRAME_UP_FRAME_MAX_SIZE] = {0};
    uint32_T n = 0;
    f32_T q[4] = {0.0f};
    static uint32_T last_ms = 0;

    now_ms = HAL_GetTick();
    /* 可以发送 */
    if(now_ms - last_ms > s_send_interval)
    {
        /* 组帧 */
        if(s_send_time_flag)
        {
            frame_buf[0] = (uint8_T)(now_ms >> 24);
            frame_buf[1] = (uint8_T)((now_ms >> 16) & (0x00ff));
            frame_buf[2] = (uint8_T)((now_ms >> 8) & (0x00ff));
            frame_buf[3] = (uint8_T)(now_ms & 0x00ff);
            n += 4;
        }

        if(s_send_dmp_quat_flag)
        { 
            get_quat(q);

            frame_buf[4] = (uint8_T)( (uint32_T)q[0] >> 24);
            frame_buf[5] = (uint8_T)(((uint32_T)q[0] >> 16) & (0x00ff));
            frame_buf[6] = (uint8_T)(((uint32_T)q[0] >> 8) & (0x00ff));
            frame_buf[7] = (uint8_T)( (uint32_T)q[0] & 0x00ff);

            frame_buf[8] = (uint8_T)(  (uint32_T)q[1] >> 24);
            frame_buf[9] = (uint8_T)(( (uint32_T)q[1] >> 16) & (0x00ff));
            frame_buf[10] = (uint8_T)(((uint32_T)q[1] >> 8) & (0x00ff));
            frame_buf[11] = (uint8_T)( (uint32_T)q[1] & 0x00ff);

            frame_buf[12] = (uint8_T)( (uint32_T)q[2] >> 24);
            frame_buf[13] = (uint8_T)(((uint32_T)q[2] >> 16) & (0x00ff));
            frame_buf[14] = (uint8_T)(((uint32_T)q[2] >> 8) & (0x00ff));
            frame_buf[15] = (uint8_T)( (uint32_T)q[2] & 0x00ff);

            frame_buf[16] = (uint8_T)( (uint32_T)q[3] >> 24);
            frame_buf[17] = (uint8_T)(((uint32_T)q[3] >> 16) & (0x00ff));
            frame_buf[18] = (uint8_T)(((uint32_T)q[3] >> 8) & (0x00ff));
            frame_buf[19] = (uint8_T)( (uint32_T)q[3] & 0x00ff);

            n += 16;
        }

        /* 发帧 */
        if(n > 0)
        {
            uart_send_bytes((drv_uart_T *)s_comm_uart, frame_buf, n);
            last_ms = now_ms;
        }
    }
   
    return;
}

/* 取反了 所以没法提取使用bit_compare */
inline static bool_T is_down_frame(uint32_T type)
{
    /* 1上行,0下行 */
    if(COMM_FRAME_DIRECTION_BIT & type) 
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

inline static bool_T bit_compare(uint32_T type, uint32_T bit_mask)
{
    /* 1有,0无 */
    if(bit_mask & type) 
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

inline static bool_T is_flyer_crtl_frame(uint32_T type)
{ 
    return bit_compare(type, COMM_FRAME_FLYER_CTRL_BIT);
}

inline static bool_T is_sensor_data_frame(uint32_T type)
{
    return bit_compare(type, COMM_FRAME_SENSOR_DATA_BIT);
}

inline static bool_T is_dmp_quat_needded(uint32_T type)
{
    return bit_compare(type, COMM_FRAME_DMP_QUAT_BIT);
}

inline static bool_T is_time_needded(uint32_T type)
{
    return bit_compare(type, COMM_FRAME_TIME_BIT);
}

/************************************* 中断 ************************************/
/*********************************** 中断句柄 **********************************/

