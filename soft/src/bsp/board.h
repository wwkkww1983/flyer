/********************************************************************************
*
* 文件名  ： board.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160624
* 版本号  ： v1.1
* 文件描述： flyer_v3.0硬件stm32f401rbt6管腿定义
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _STM32F4XX_HAL_MSP_H_
#define _STM32F4XX_HAL_MSP_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal.h"

/************************************ 宏定义 ***********************************/
/************************************  MLED  ***********************************/
/* 
 * 使用PB4
 * */
#define MLED                                    GPIOB
#define MLED_CLK_ENABLE()                       __HAL_RCC_GPIOB_CLK_ENABLE()
#define MLED_GPIO_PIN                           GPIO_PIN_4

/************************************  PWM   ***********************************/
/* 
 * 使用TIM2 
 * PCB位置  PWM编号   通道  管腿
 * 后       PWM3      CH1   PA0
 * 左       PWM4      CH2   PB3
 * 前       PWM1      CH3   PB10
 * 右       PWM2      CH4   PA3
 * */
#define PWM_TIM                                 TIM2
#define PWM_TIM_CLK_ENABLE()                    __HAL_RCC_TIM2_CLK_ENABLE()
#define PWM_TIM_CHANNEL1_PORT_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define PWM_TIM_CHANNEL2_PORT_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define PWM_TIM_CHANNEL3_PORT_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define PWM_TIM_CHANNEL4_PORT_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define PWM_TIM_GPIO_PORT_CHANNEL1              GPIOA
#define PWM_TIM_GPIO_PORT_CHANNEL2              GPIOB
#define PWM_TIM_GPIO_PORT_CHANNEL3              GPIOB
#define PWM_TIM_GPIO_PORT_CHANNEL4              GPIOA
#define PWM_TIM_GPIO_PIN_CHANNEL1               GPIO_PIN_0
#define PWM_TIM_GPIO_PIN_CHANNEL2               GPIO_PIN_3
#define PWM_TIM_GPIO_PIN_CHANNEL3               GPIO_PIN_10
#define PWM_TIM_GPIO_PIN_CHANNEL4               GPIO_PIN_3
#define PWM_TIM_GPIO_AF_CHANNEL1                GPIO_AF1_TIM2
#define PWM_TIM_GPIO_AF_CHANNEL2                GPIO_AF1_TIM2
#define PWM_TIM_GPIO_AF_CHANNEL3                GPIO_AF1_TIM2
#define PWM_TIM_GPIO_AF_CHANNEL4                GPIO_AF1_TIM2

/****************************** 串口: 控制台+ESP8266 ***************************/
/************************************* 控制台 **********************************/
/*
 * USART1
 * TX:PA9
 * RX:PA10
 * */
#define CONSOLE_UART                            USART1
#define CONSOLE_UART_CLK_ENABLE()               __HAL_RCC_USART1_CLK_ENABLE()
#define CONSOLE_UART_RX_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define CONSOLE_UART_TX_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define CONSOLE_UART_FORCE_RESET()              __HAL_RCC_USART1_FORCE_RESET()
#define CONSOLE_UART_RELEASE_RESET()            __HAL_RCC_USART1_RELEASE_RESET()
#define CONSOLE_UART_TX_GPIO_PORT               GPIOA
#define CONSOLE_UART_TX_PIN                     GPIO_PIN_9
#define CONSOLE_UART_TX_AF                      GPIO_AF7_USART1
#define CONSOLE_UART_RX_GPIO_PORT               GPIOA
#define CONSOLE_UART_RX_PIN                     GPIO_PIN_10
#define CONSOLE_UART_RX_AF                      GPIO_AF7_USART1
#define CONSOLE_UART_IRQn                       USART1_IRQn
#define CONSOLE_UART_IRQHANDLER                 USART1_IRQHandler
#define CONSOLE_TX_DMA_CLK_ENABLE()             __HAL_RCC_DMA2_CLK_ENABLE()
#define CONSOLE_UART_TX_DMA_CHANNEL             DMA_CHANNEL_4
#define CONSOLE_UART_TX_DMA_STREAM              DMA2_Stream7
#define CONSOLE_UART_DMA_TX_IRQn                DMA2_Stream7_IRQn
#define CONSOLE_UART_DMA_TX_IRQHandler          DMA2_Stream7_IRQHandler
#define CONSOLE_RX_DMA_CLK_ENABLE()             __HAL_RCC_DMA2_CLK_ENABLE()
#define CONSOLE_UART_RX_DMA_CHANNEL             DMA_CHANNEL_4
#define CONSOLE_UART_RX_DMA_STREAM              DMA2_Stream5
#define CONSOLE_UART_DMA_RX_IRQn                DMA2_Stream5_IRQn
#define CONSOLE_UART_DMA_RX_IRQHandler          DMA2_Stream5_IRQHandler

/******************************* ESP82699串口定义 ******************************/
/*
 * USART6
 * TX:PC6
 * RX:PC7
 * */
#define ESP8266_UART                            USART6
#define ESP8266_UART_CLK_ENABLE()               __HAL_RCC_USART6_CLK_ENABLE()
#define ESP8266_UART_RX_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOC_CLK_ENABLE()
#define ESP8266_UART_TX_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOC_CLK_ENABLE()
#define ESP8266_UART_FORCE_RESET()              __HAL_RCC_USART6_FORCE_RESET()
#define ESP8266_UART_RELEASE_RESET()            __HAL_RCC_USART6_RELEASE_RESET()
#define ESP8266_UART_TX_GPIO_PORT               GPIOC
#define ESP8266_UART_TX_PIN                     GPIO_PIN_6
#define ESP8266_UART_TX_AF                      GPIO_AF8_USART6
#define ESP8266_UART_RX_GPIO_PORT               GPIOC
#define ESP8266_UART_RX_PIN                     GPIO_PIN_7
#define ESP8266_UART_RX_AF                      GPIO_AF8_USART6
#define ESP8266_UART_IRQn                       USART6_IRQn
#define ESP8266_UART_IRQHANDLER                 USART6_IRQHandler
#define ESP8266_TX_DMA_CLK_ENABLE()             __HAL_RCC_DMA2_CLK_ENABLE()
#define ESP8266_UART_TX_DMA_CHANNEL             DMA_CHANNEL_5
#define ESP8266_UART_TX_DMA_STREAM              DMA2_Stream6
#define ESP8266_UART_DMA_TX_IRQn                DMA2_Stream6_IRQn
#define ESP8266_UART_DMA_TX_IRQHandler          DMA2_Stream6_IRQHandler
#define ESP8266_RX_DMA_CLK_ENABLE()             __HAL_RCC_DMA2_CLK_ENABLE()
#define ESP8266_UART_RX_DMA_CHANNEL             DMA_CHANNEL_5
#define ESP8266_UART_RX_DMA_STREAM              DMA2_Stream2
#define ESP8266_UART_DMA_RX_IRQn                DMA2_Stream2_IRQn
#define ESP8266_UART_DMA_RX_IRQHandler          DMA2_Stream2_IRQHandler

/********************************** SENSOR I2C 定义 ****************************/
/*
 * I2C1
 * SCL:PC6
 * SDA:PC7
 * */
#define SENSOR_I2C                              I2C1
#define SENSOR_I2C_CLOCK_ENABLE()               __I2C1_CLK_ENABLE()
#define SENSOR_I2C_FORCE_RESET()                __I2C1_FORCE_RESET()
#define SENSOR_I2C_RELEASE_RESET()              __I2C1_RELEASE_RESET()
#define SENSOR_I2C_SDA_GPIO_CLK_ENABLE()        __GPIOB_CLK_ENABLE()
#define SENSOR_I2C_SCL_GPIO_CLK_ENABLE()        __GPIOB_CLK_ENABLE() 
#define SENSOR_I2C_SDA_GPIO_CLK_DISABLE()       __GPIOB_CLK_DISABLE()
#define SENSOR_I2C_SCL_GPIO_CLK_DISABLE()       __GPIOB_CLK_DISABLE() 
#define SENSOR_I2C_SCL_GPIO_PORT                GPIOB
#define SENSOR_I2C_SCL_PIN                      GPIO_PIN_8
#define SENSOR_I2C_SCL_AF                       GPIO_AF4_I2C1
#define SENSOR_I2C_SDA_GPIO_PORT                GPIOB
#define SENSOR_I2C_SDA_PIN                      GPIO_PIN_9
#define SENSOR_I2C_SDA_AF                       GPIO_AF4_I2C1
#define SENSOR_I2C_EV_IRQn                      I2C1_EV_IRQn
#define SENSOR_I2C_EV_IRQHandler                I2C1_EV_IRQHandler                                            
#define SENSOR_I2C_ER_IRQn                      I2C1_ER_IRQn
#define SENSOR_I2C_ER_IRQHandler                I2C1_ER_IRQHandler
#define SENSOR_I2C_DMA_CLK_ENABLE()             __HAL_RCC_DMA1_CLK_ENABLE()
#define SENSOR_I2C_RX_DMA_CHANNEL               DMA_CHANNEL_1
#define SENSOR_I2C_RX_DMA_STREAM                DMA1_Stream5
#define SENSOR_I2C_DMA_RX_IRQn                  DMA1_Stream5_IRQn
#define SENSOR_I2C_DMA_RX_IRQHandler            DMA1_Stream5_IRQHandler

/*
 * EXIT13
 * PC13
 * */
#define SENSOR_INT_PIN                          GPIO_PIN_13
#define SENSOR_INT_GPIO_PORT                    GPIOC
#define SENSOR_INT_MODE                         GPIO_MODE_IT_FALLING
#define SENSOR_INT_CLK_ENABLE()                 __GPIOC_CLK_ENABLE()
#define SENSOR_INT_EXTI                         EXTI15_10_IRQn
#define SENSOR_INT_EXTI_IRQHandler              EXTI15_10_IRQHandler

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void clock_init(void);

#endif /* _STM32F4XX_HAL_MSP_H_ */

