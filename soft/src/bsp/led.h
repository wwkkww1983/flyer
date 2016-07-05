/********************************************************************************
*
* 文件名  ： led.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160705
* 版本号  ： v1.1
* 文件描述： led模块
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _LED_H_
#define _LED_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"
#include "led.h"

/************************************ 宏定义 ***********************************/

/*********************************** 类型定义 **********************************/
/* led硬件 */
typedef struct led_list_tag
{
    GPIO_TypeDef* port; /* PORT: A B C ...... */
    uint32_T pin;       /* PIN : 0 1 2 3 ..... */
}LED_LIST_T;

/* LED名字 编号 */
typedef enum
{
    LED_MLED = 0,
    LED_MAX
}LED_NAME;

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/
extern LED_LIST_T g_led_list[];

/*********************************** 接口函数 **********************************/
void led_init(void);
void led_on(LED_NAME led);
void led_off(LED_NAME led);
void led_toggle(LED_NAME led);
void led_test(void);

#endif

