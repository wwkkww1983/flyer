/******************************************************************************
 *
 * 文件名  ： esp8266.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20160624 
 * 版本号  ： 1.0
 * 文件描述： 控制台模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 输入 逐个字符
 *            输出 按行
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typedef.h"
#include "config.h"
#include "board.h"
#include "uart.h"
#include "esp8266.h"
#include "console.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/
/* board.c中配置DMA需要使用 所以不能定义为static */
drv_uart_T g_esp8266 = {
    .reg_base = ESP8266_UART,
    .baud_rate = ESP8266_BAUDRATE,
};

static esp8266_cmd_T s_cmd_list[] = {
    {"ATE0\r\n", "ATE0\r\r\n\r\nOK\r\n", 1},                                    /* 关闭回显 */
    {"AT+CWMODE=1\r\n", "\r\nOK\r\n", 1},                                       /* 设置STA共存模式 */ 
    
    /* 该命令不检查回显(耗时较长,未完成的话会返回busy,AT+CIPMUX=0会检查) */
    {"AT+CWJAP=\"pc4esp8266\",\"pp866158\"\r\n", "\n", 1},                      /* 加入路由器,与服务器在同一局域网下,该命令需要连接AP 比较费时 */
    {"AT+CIPMUX=0\r\n", "\r\nOK\r\n", 1},                                       /* 设置单链接 */
    {"AT+CIPMODE=1\r\n", "\r\nOK\r\n", 1},                                      /* 设置透传模式 */
    {"AT+CIPSTART=\"UDP\",\"192.168.1.2\",8080\r\n", "CONNECT\r\n\r\nOK\r\n", 1},/* 正常udp连接测试(IP和端口为PC服务器的) */
    {"AT+CIPSTATUS\r\n", "STATUS:2\r\n", 1},                                    /* 获取连接状态 */
    //{"AT+CIFSR\r\n", "\r\nOK\r\n", 1},                                        /* 获取本地ip状态 */
    //{"AT+CIPSTART=0,\"UDP\",\"255.255.255.255\",1000,50000,1", "\r\nOK\r\n", 1}, /* 启动udp服务器 */
    {"AT+CIPSEND\r\n", ">", 1},                                                 /* 启动透传 */
    {NULL, NULL}
};

/********************************** 函数声明区 *********************************/
static void esp8266_reset(void); 
static void esp8266_wati_reset_ok(void);

/********************************** 函数实现区 *********************************/
/* 控制台初始化 */
void esp8266_init(void)
{ 
    uint32_T i = 0;
    uint8_T *cmd = NULL;
    uint8_T *echo = NULL; 
    
    uint8_t recv_buf[ESP8266_BUF_SIZE] = {0};

    uart_init(&g_esp8266); 
    
    /* step1: 复位esp8266 */
    esp8266_reset(); 

    /* step2: 设置模式 */ 
    do
    {
        cmd = s_cmd_list[i].cmd;
        echo = s_cmd_list[i].echo;
        if(NULL == cmd)
        {
            break;
        }

        do
        { 
            memset(recv_buf, NUL, ESP8266_BUF_SIZE);

            /* 输出AT命令 */
            uart_send_bytes_poll(&g_esp8266, cmd, strlen((const char*)cmd));

            /* 阻塞等待esp8266回显 */
            uart_recv_bytes_poll(&g_esp8266, recv_buf, ESP8266_BUF_SIZE); 

#if 0
            /* 检查命令是否成功 */
            if(0 == i)
            {
                int j = 0;
                j = 1;
            }
#endif
        
            /* 检查回显 确认设置成功 */
            if(NULL != strstr((const char *)recv_buf, (const char *)echo))
            {
                break;
            }
            else /* 回显错误 延迟后重发命令 */
            { 
                /* 延迟 避免出问题 */
                HAL_Delay(s_cmd_list[i].delay);

#if 0
                /* 统计命令列表中命令 错误数 */
                uint32_T cmd_error_times[10] = {0};
                cmd_error_times[i]++;
#endif
            }
        }while(1);

        i++;

    }while(1); 
}

/* 复位esp8266模块 */
static void esp8266_reset(void)
{
    HAL_GPIO_WritePin(ESP8266_PWR_PORT, ESP8266_PWR_GPIO_PIN, GPIO_PIN_RESET); 
    HAL_Delay(ESP8266_DELAY);
    HAL_GPIO_WritePin(ESP8266_PWR_PORT, ESP8266_PWR_GPIO_PIN, GPIO_PIN_SET); 

    /* 确认esp8266复位成功 */
    esp8266_wati_reset_ok();
}

/* esp8266测试 */
void esp8266_test(void)
{
    debug_log("观察esp8266并按照提示操作.\r\n");
}

void esp8266_wati_reset_ok(void)
{ 
    uint8_t ch = 0;
    uint8_t recv_buf[ESP8266_BUF_SIZE] = {0};
    while(1)
    { 
        uart_recv_bytes_poll(&g_esp8266, &ch, 1);

        /* 阻塞 等待数据 */
        /* uart_recv_bytes(&g_esp8266, s_debug_buf, DEBUG_BUF_SIZE);
        while(!uart_frame_ready(&g_esp8266)); */
        
        /* esp8266启动乱码后会输出\r\n,之后输出公司信息,公司信息后面带有"\r\nready\r\n"表示启动完成 */
        /* 只考虑'\n' 之后的内容 */
        if('\n' == ch) /* 找到 */
        { 
            /* 接收后面连续5个字符 */
            uart_recv_bytes_poll(&g_esp8266, recv_buf, ESP8266_READY_STR_SIZE);
            if(0 == strncmp((const char *)recv_buf, ESP8266_READY_STR, ESP8266_READY_STR_SIZE)) /* 找到 */
            {
                break;
            }
            else /* 没有找到 复位esp8266 重新找 */
            { 
                /* 经验表明不会到此分支 */
                while(1);
                //esp8266_reset(); 
            }
        }
    }
}

/************************************* 中断 ************************************/
/*********************************** 中断句柄 **********************************/
void ESP8266_UART_IRQHANDLER(void)
{
    HAL_UART_IRQHandler(&(g_esp8266.handle));
}

void ESP8266_UART_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler((g_esp8266.handle).hdmatx);
}

void ESP8266_UART_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler((g_esp8266.handle).hdmarx);
}

