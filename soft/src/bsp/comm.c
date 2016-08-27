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
#include <string.h>
#include <stm32f4xx_hal.h>

#include "board.h"
#include "uart.h"
#include "esp8266.h"
#include "console.h"
#include "mpu9250.h"
#include "pwm.h"
#include "ctrl.h"
#include "debug.h"
#include "comm.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
const drv_uart_T *s_comm_uart = NULL;
static CRC_HandleTypeDef s_crc;

static uint32_T s_send_interval = 0;
static bool_T s_send_time_flag = FALSE;
static bool_T s_send_accelerator_flag = FALSE;
static bool_T s_send_dmp_quat_flag = FALSE;
static bool_T s_send_euler_flag = FALSE;
static bool_T s_send_pid_flag = FALSE;
//static const uint8_T *s_hello = "flyer ok.\r\n"; 
static uint8_T s_recv_frame_buf[COMM_DOWN_FRAME_BUF_SIZE] = {0};

/********************************** 函数声明区 *********************************/
static bool_T parse(const uint8_T *frame);
static void send_capture_data(void);
inline static bool_T is_down_frame(uint32_T type);
inline static bool_T bit_compare(uint32_T type, uint32_T bit_mask);
inline static bool_T is_flyer_ctrl_frame(uint32_T type);
inline static bool_T is_sensor_data_frame(uint32_T type);
inline static bool_T is_dmp_quat_needded(uint32_T type);
inline static bool_T is_time_needded(uint32_T type);
inline static bool_T is_acceletorater_needded(uint32_T type);
inline static bool_T is_euler_needded(uint32_T type);
inline static bool_T is_pid_needded(uint32_T type);
inline static bool_T is_pid_frame(uint32_T type);
//static void comm_wait_start(void);

/********************************** 函数实现区 *********************************/
void comm_init(const drv_uart_T *comm_uart)
{
    /* 配置 协议走的通道 console or esp8266 */
    if(NULL == comm_uart)
    {
        ERR_STR("参数错误.");
    }
    s_comm_uart =  comm_uart;

    /* 配置硬件CRC32 */
    s_crc.Instance = CRC32;
    CRC_CLK_ENABLE();
    if(HAL_OK != HAL_CRC_Init(&s_crc))
    {
        ERR_STR("函数执行失败.");
    } 
    
    /* 初始化中 启动串口接收 */
    /* 启动下行帧接收 */
    uart_recv_bytes((drv_uart_T *)s_comm_uart, s_recv_frame_buf, COMM_DOWN_FRAME_BUF_SIZE);

    /* 阻塞等待启动信号 */
    //comm_wait_start();
}

#if 0
static void comm_wait_start(void)
{
    uint8_T frame_buf[COMM_DOWN_FRAME_BUF_SIZE] = {0}; 
    bool_T frame_is_valid = FALSE; 
    
    /* 启动上位机帧接收 */
    uart_recv_bytes((drv_uart_T *)s_comm_uart, frame_buf, COMM_DOWN_FRAME_BUF_SIZE); 

    /* TODO:完善握手协议 */
    while(TRUE)
    { 

        /* 发送握手(告诉上位机,目前在等待,发送合法启动帧) */
        uart_send_bytes_poll((drv_uart_T *)s_comm_uart, (uint8_T *)s_hello, strlen((const char*)s_hello)); 

        HAL_Delay(COMM_SEND_HELLO_DELAY);

        /* 阻塞等待接收就绪 */
        if(!uart_frame_ready(s_comm_uart))
        {
            continue;
        }
        else 
        {
            frame_is_valid = parse(frame_buf); 
            if(frame_is_valid) /* 合法帧 */ 
            {
                break;
            }
            else /* 接收到上位机非法帧 重新等待合法帧 */
            { 
                /* 启动上位机帧接收 */
                uart_recv_bytes((drv_uart_T *)s_comm_uart, frame_buf, COMM_DOWN_FRAME_BUF_SIZE); 
                continue;
            }
        }
    }

    /* 用于调试 */
		/* frame_is_valid = TRUE;*/

#if 0
    /* 启动上位机帧接收 */
    uart_recv_bytes((drv_uart_T *)s_comm_uart, frame_buf, COMM_DOWN_FRAME_BUF_SIZE); 

    /* 发送握手(告诉上位机,目前在等待,发送合法启动帧) */
    uart_send_bytes_poll((drv_uart_T *)s_comm_uart, (uint8_T *)s_hello, strlen((const char*)s_hello)); 

    do
    { 
        HAL_Delay(200);
        frame_ready = uart_frame_ready(s_comm_uart);
    }while(!frame_ready);

    parse(frame_buf); 
#endif

#if 0
    while(TRUE)
    {
        /* 启动下行帧接收 */
        uart_recv_bytes((drv_uart_T *)s_comm_uart, frame_buf, COMM_DOWN_FRAME_BUF_SIZE); 
        
        do
        {
            /* 发送握手(告诉上位机,目前在等待,发送合法启动帧) */
            uart_send_bytes_poll((drv_uart_T *)s_comm_uart, (uint8_T *)s_hello, strlen((const char*)s_hello)); 
            frame_ready = uart_frame_ready(s_comm_uart);
        }while(!frame_ready); /* 阻塞等待上位机合法帧 */ 
        
        /* 解析 */
        frameIsValid = parse(frame_buf); 
        if(frameIsValid) /* 合法帧 */ 
        {
            break;
        }
        else
        {
            continue;
        }
    }
#endif
}
#endif

/* 通信交互任务 */
void comm_update(void)
{ 
    send_capture_data(); 
    
    /* 第一帧在comm_init中启动接收 等待上一帧接收完成 */
    if(!uart_frame_ready(s_comm_uart))
    {
        /* 帧未到达 退出(等待下一轮) */
        return;
    }
    else
    { 
        /* 解析(包含处理) */
        parse(s_recv_frame_buf);

        /* 解析后清零 避免多次解析 */
        memset(s_recv_frame_buf, 0, COMM_DOWN_FRAME_BUF_SIZE);

        /* 启动下行帧接收 */
        uart_recv_bytes((drv_uart_T *)s_comm_uart, s_recv_frame_buf, COMM_DOWN_FRAME_BUF_SIZE);
    }
}

static bool_T parse(const uint8_T *buf)
{ 
    bool_T has_capture_data = FALSE;
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

    frame.crc   = buf[COMM_DOWN_FRAME_BUF_SIZE - 4] << 24;
    frame.crc  |= buf[COMM_DOWN_FRAME_BUF_SIZE - 3] << 16;
    frame.crc  |= buf[COMM_DOWN_FRAME_BUF_SIZE - 2] << 8;
    frame.crc  |= buf[COMM_DOWN_FRAME_BUF_SIZE - 1];

    /* 长度检查 固定长度 data+crc32 8Bytes */
    if(COMM_DOWN_FRAME_DATA_AND_CRC32_SIZE != frame.len) /* 错误帧 不处理 */
    {
        return FALSE;
    }

    /* CRC检查 */ 
    crc32_calculated = HAL_CRC_Calculate(&s_crc, (uint32_T *)buf, COMM_DOWN_FRAME_BUF_SIZE / sizeof(uint32_T) - 1); 
    if(crc32_calculated != frame.crc) /* 校验失败*/
    {
        return FALSE;
    }

    /* 类型检查 */ 
    if(!is_down_frame(frame.type)) /* 错 收到上行帧 */
    {
        return FALSE;
    }
   
    /* 采样请求帧 */
    if(is_sensor_data_frame(frame.type))
    { 
        /* 发送时间 */
        if(is_time_needded(frame.type))
        {
            s_send_interval  = buf[8] << 24;
            s_send_interval |= buf[9] << 16;
            s_send_interval |= buf[10] << 8;
            s_send_interval |= buf[11]; 
            
            /* 限制时间间隔(无符号32位 必然大于0不用比下界) */
            if(s_send_interval > COMM_FRAME_INTERVAL_MAX)
            {
                s_send_interval = COMM_FRAME_INTERVAL_MAX;
            } 
            
            s_send_time_flag = TRUE;
            has_capture_data = TRUE;
        }

        /* 发送四元数 */
        if(is_dmp_quat_needded(frame.type))
        {
            s_send_dmp_quat_flag = TRUE;
            has_capture_data = TRUE;
        }

        /* 发送油门 */
        if(is_acceletorater_needded(frame.type))
        {
            s_send_accelerator_flag = TRUE;
            has_capture_data = TRUE;
        }

        /* 发送欧拉角 */
        if(is_euler_needded(frame.type))
        {
            s_send_euler_flag = TRUE;
            has_capture_data = TRUE;
        }

        /* 发送PID */
        if(is_pid_needded(frame.type))
        {
            s_send_pid_flag = TRUE;
            has_capture_data = TRUE;
        }

        return has_capture_data;
    }

    /* PID参数设置帧 */
    if(is_pid_frame(frame.type))
    {
        int32_T ctrl_type = buf[8];

        switch(ctrl_type)
        {
            case CTRL_THETA:
                return TRUE;
            case CTRL_PHI:
                return TRUE;
            case CTRL_PSI:
                return TRUE;
            default:
                return FALSE;
        }

    }

    /* 飞控帧 */
    if(is_flyer_ctrl_frame(frame.type))
    {
        int32_T ctrl_type = buf[8];
        int32_T val[PWM_MAX] = {0};

        /* return 替代break */
        switch(ctrl_type)
        {
            /* 熄火 */
            case 0x00: 
                { 
                    ctrl_motor_off();
                    return TRUE;
                }

            /* 油门 */
            case 0x01: 
                {
                    for(int32_T i = 0; i < PWM_MAX; i++)
                    {
                        val[i]  = buf[9] << 8;
                        val[i] |= buf[10];
                    }
                    ctrl_set_acceleralor(val);
                    return TRUE;
                }
            /* 错误类型 */
            default:
                return FALSE;
        }
    }

    return FALSE;
}

/* 发送采样数据 */
static void send_capture_data(void)
{
    uint32_T type = 0;

    /* 避免函数退出 栈内存被破坏(DMA传输 要求内存数据保持) */
    static uint8_T frame_buf[COMM_FRAME_CAPTURE_FRAME_MAX_SIZE] = {0};
    uint32_T len = 0;

    uint32_T i = 0;
    uint32_T fill_bytes_count = 0;
    uint32_T crc32_calculated = 0;
    uint32_T now_ms = 0;
    f32_T quat[4] = {0.0f}; 
    f32_T euler[CTRL_EULER_MAX] = {0.0f}; 
    f32_T pid_out[CTRL_EULER_MAX] = {0.0f}; 
    int32_T accelerator_max = 0;
    int32_T accelerator[PWM_MAX] = {0};
    uint32_T *p_ui32 = NULL;
    static uint32_T last_ms = 0;

    /* 无帧可发 */
    if(!(s_send_interval
    || s_send_time_flag
    || s_send_accelerator_flag
    || s_send_dmp_quat_flag
    || s_send_euler_flag
    || s_send_pid_flag))
    {
        return;
    }

    /* 可以发送 */
    now_ms = HAL_GetTick();
    if(now_ms - last_ms > s_send_interval)
    {
        /* 上行且有传感数据长度32 */
        type = COMM_FRAME_SENSOR_DATA_BIT
             | COMM_FRAME_DIRECTION_BIT;

        /* 跳过type和len域 */
        len += 8;

        /* 组帧 */
        if(s_send_time_flag)
        {
            frame_buf[len++] = (uint8_T)(now_ms >> 24);
            frame_buf[len++] = (uint8_T)((now_ms >> 16));
            frame_buf[len++] = (uint8_T)((now_ms >> 8));
            frame_buf[len++] = (uint8_T)(now_ms); 

            type |= COMM_FRAME_TIME_BIT;
        }

        if(s_send_dmp_quat_flag)
        {
            mpu9250_get_quat(quat);
#if 0
            /* 调试上位机绘图基准 */
            quat[0] = 1.0f;
            quat[1] = 0.0f;
            quat[2] = 0.0f;
            quat[3] = 0.0f;
#endif
            p_ui32 = (uint32_T *)quat;
            for(i = 0; i < 4; i++) 
            {
                frame_buf[len++] = (uint8_T)( (uint32_T)p_ui32[i] >> 24);
                frame_buf[len++] = (uint8_T)(((uint32_T)p_ui32[i] >> 16));
                frame_buf[len++] = (uint8_T)(((uint32_T)p_ui32[i] >> 8));
                frame_buf[len++] = (uint8_T)( (uint32_T)p_ui32[i]);
            }

            type |= COMM_FRAME_DMP_QUAT_BIT;
        } 

        /* 油门数据 */
        if(s_send_accelerator_flag)
        { 
            ctrl_get_acceleralor(accelerator, &accelerator_max);

            for(i = 0; i < PWM_MAX; i++) 
            { 
                frame_buf[len++] = (uint8_T)(accelerator[i] >> 24);
                frame_buf[len++] = (uint8_T)(accelerator[i] >> 16);
                frame_buf[len++] = (uint8_T)(accelerator[i] >> 8);
                frame_buf[len++] = (uint8_T)(accelerator[i]);
            } 
            
            frame_buf[len++] = (uint8_T)(accelerator_max >> 24);
            frame_buf[len++] = (uint8_T)(accelerator_max >> 16);
            frame_buf[len++] = (uint8_T)(accelerator_max >> 8);
            frame_buf[len++] = (uint8_T)(accelerator_max);

            type |= COMM_FRAME_ACCELERATOR_DATA_BIT;
        }

        /* 欧拉角 */
        if(s_send_euler_flag)
        { 
            mpu9250_get_quat(quat);
            math_quaternion2euler(euler, quat);

            p_ui32 = (uint32_T *)euler;
            for(i = 0; i < CTRL_EULER_MAX; i++) 
            {
                frame_buf[len++] = (uint8_T)(p_ui32[i] >> 24);
                frame_buf[len++] = (uint8_T)(p_ui32[i] >> 16);
                frame_buf[len++] = (uint8_T)(p_ui32[i] >> 8);
                frame_buf[len++] = (uint8_T)(p_ui32[i]);
            }

            type |= COMM_FRAME_EULER_DATA_BIT;
        }

        /* pid数据 */
        if(s_send_pid_flag)
        { 
            ctrl_get_pid_out(pid_out);
            p_ui32 = (uint32_T *)pid_out;
            for(i = 0; i < CTRL_EULER_MAX; i++) 
            {
                frame_buf[len++] = (uint8_T)(p_ui32[i] >> 24);
                frame_buf[len++] = (uint8_T)(p_ui32[i] >> 16);
                frame_buf[len++] = (uint8_T)(p_ui32[i] >> 8);
                frame_buf[len++] = (uint8_T)(p_ui32[i]);
            }

            type |= COMM_FRAME_PID_DATA_BIT;
        }

        /* 填充 */
        fill_bytes_count = len & 0x03;
        for(i = 0; i < fill_bytes_count; i++)
        {
            frame_buf[len++] = COMM_FRAME_FILLED_VAL;
        }

        /* 填写type域 */
        frame_buf[0] = (uint8_T)(type >> 24);
        frame_buf[1] = (uint8_T)(type >> 16);
        frame_buf[2] = (uint8_T)(type >> 8);
        frame_buf[3] = (uint8_T)(type);

        /* 计入crc长度 */
        len += 4;
				len -= 8; /* len中长度值不包括type+len */
        /* 填写len域 */
        frame_buf[4] = (uint8_T)(len >> 24);
        frame_buf[5] = (uint8_T)(len >> 16);
        frame_buf[6] = (uint8_T)(len >> 8);
        frame_buf[7] = (uint8_T)(len);
        
        /* len偏移到crc位置 */
        len += 4;
        /* 计算校验 */
        crc32_calculated = HAL_CRC_Calculate(&s_crc, (uint32_T *)frame_buf, len / 4); 
        frame_buf[len++] = (uint8_T)(crc32_calculated >> 24);
        frame_buf[len++] = (uint8_T)(crc32_calculated >> 16);
        frame_buf[len++] = (uint8_T)(crc32_calculated >> 8);
        frame_buf[len++] = (uint8_T)(crc32_calculated);

        /* 发帧 */
        if(len > COMM_FRAME_SENDED_MIN)
        {
#if 0
            /* 出错 用于检查 crc是否为0 */
            if(0 == frame_buf[n-1])
            {
                ERR_STR("crc错误.");
            }
#endif
            uart_send_bytes((drv_uart_T *)s_comm_uart, frame_buf, len);
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

inline static bool_T is_flyer_ctrl_frame(uint32_T type)
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

inline static bool_T is_acceletorater_needded(uint32_T type)
{
    return bit_compare(type, COMM_FRAME_ACCELERATOR_DATA_BIT);
}

inline static bool_T is_euler_needded(uint32_T type)
{
    return bit_compare(type, COMM_FRAME_EULER_DATA_BIT);
}

inline static bool_T is_pid_needded(uint32_T type)
{
    return bit_compare(type, COMM_FRAME_PID_DATA_BIT);
}

inline static bool_T is_pid_frame(uint32_T type)
{ 
    return bit_compare(type, COMM_FRAME_PID_CTRL_BIT);
}

/* 提取构帧逻辑(type/len/crc/填充生成) */
void comm_frame_printf_make(uint32_T *frame_len, uint8_T *frame_buf, uint32_T n)
{ 
    uint32_T type = 0;
    uint32_T len = 0;
    uint32_T fill_bytes_count = 0;
    uint32_T crc32_calculated = 0;
    uint32_T buf_index = 0;
    uint32_T i = 0;
    uint32_T left = 0;

    /* 需要填充的字节数: &0x03 等效于 %4 */
    left = n % 4;
    if(0 != left) /* 需要填充 */
    { 
        fill_bytes_count = 4 - left;
    }

    /* 必然是4的倍数 */
    if(0 != (n + fill_bytes_count) % 4)
    {
        ERR_STR("填充数据错误.");
    }

    type = COMM_FRAME_DIRECTION_BIT
         | COMM_FRAME_PRINTF_BIT;
    len = n + fill_bytes_count + sizeof(int); /* 数据+填充+crc32 */

    frame_buf[buf_index++] = (uint8_T)(type >> 24);
    frame_buf[buf_index++] = (uint8_T)(type >> 16);
    frame_buf[buf_index++] = (uint8_T)(type >> 8);
    frame_buf[buf_index++] = (uint8_T)(type); 
    
    frame_buf[buf_index++] = (uint8_T)(len >> 24);
    frame_buf[buf_index++] = (uint8_T)(len >> 16);
    frame_buf[buf_index++] = (uint8_T)(len >> 8);
    frame_buf[buf_index++] = (uint8_T)(len); 

    /* 填充 */
    buf_index += n; /* 跳过打印数据 */
    for(i = 0; i < fill_bytes_count; i++)
    {
        frame_buf[buf_index++] = COMM_FRAME_FILLED_VAL;
    }

    crc32_calculated = HAL_CRC_Calculate(&s_crc, (uint32_T *)frame_buf, buf_index / sizeof(int)); 
    frame_buf[buf_index++] = (uint8_T)(crc32_calculated >> 24);
    frame_buf[buf_index++] = (uint8_T)(crc32_calculated >> 16);
    frame_buf[buf_index++] = (uint8_T)(crc32_calculated >> 8);
    frame_buf[buf_index++] = (uint8_T)(crc32_calculated);

    *frame_len = buf_index;
}

/************************************* 中断 ************************************/
/*********************************** 中断句柄 **********************************/

