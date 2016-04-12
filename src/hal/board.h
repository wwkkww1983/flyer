/********************************************************************************
*
* 文件名  ： uart.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150615
* 版本号  ： v1.0
* 文件描述： stm32f4 cube hal 初始化回调头文件
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _STM32F4XX_HAL_MSP_H_
#define _STM32F4XX_HAL_MSP_H_

/************************************ 头文件 ***********************************/
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal.h"

/************************************ 宏定义 ***********************************/
/* LED1 定义 PC5 */
#define LED1x                                   GPIOC
#define LED1x_CLK_ENABLE()                      __HAL_RCC_GPIOC_CLK_ENABLE(); 
#define LED1x_GPIO_PIN                          GPIO_PIN_5
/* LED2 定义 PC4 */
#define LED2x                                   GPIOC
#define LED2x_CLK_ENABLE()                      __HAL_RCC_GPIOC_CLK_ENABLE(); 
#define LED2x_GPIO_PIN                          GPIO_PIN_4
/* LED3 定义 PA5 */
#define LED3x                                   GPIOA
#define LED3x_CLK_ENABLE()                      __HAL_RCC_GPIOA_CLK_ENABLE(); 
#define LED3x_GPIO_PIN                          GPIO_PIN_5
/* LED4 定义 PB12 */
#define LED4x                                   GPIOB
#define LED4x_CLK_ENABLE()                      __HAL_RCC_GPIOB_CLK_ENABLE(); 
#define LED4x_GPIO_PIN                          GPIO_PIN_12

/* PWM个数 1个四通道 */
#define PWM_NUMS                                (1)
/* PWM1 定义 */
#define TIMx                                    TIM1
#define TIMx_CLK_ENABLE()                       __HAL_RCC_TIM1_CLK_ENABLE()
#define TIMx_CHANNEL_GPIO_PORT()                __HAL_RCC_GPIOA_CLK_ENABLE();
#define TIMx_GPIO_PORT_CHANNEL1                 GPIOA
#define TIMx_GPIO_PORT_CHANNEL2                 GPIOA
#define TIMx_GPIO_PORT_CHANNEL3                 GPIOA
#define TIMx_GPIO_PORT_CHANNEL4                 GPIOA
#define TIMx_GPIO_PIN_CHANNEL1                  GPIO_PIN_8
#define TIMx_GPIO_PIN_CHANNEL2                  GPIO_PIN_9
#define TIMx_GPIO_PIN_CHANNEL3                  GPIO_PIN_10
#define TIMx_GPIO_PIN_CHANNEL4                  GPIO_PIN_11
#define TIMx_GPIO_AF_CHANNEL1                   GPIO_AF1_TIM1
#define TIMx_GPIO_AF_CHANNEL2                   GPIO_AF1_TIM1
#define TIMx_GPIO_AF_CHANNEL3                   GPIO_AF1_TIM1
#define TIMx_GPIO_AF_CHANNEL4                   GPIO_AF1_TIM1
/* TODO: 扩展PWM定义 */

/* 串口个数 控制台+ESP8266 */
#define UART_NUMS                               (1)
/* ESP82699串口定义 */
#define CONSOLE_UART                            USART1
#define CONSOLE_UART_CLK_ENABLE()               __HAL_RCC_USART1_CLK_ENABLE()
#define CONSOLE_UART_RX_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define CONSOLE_UART_TX_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define CONSOLE_UART_FORCE_RESET()              __HAL_RCC_USART1_FORCE_RESET()
#define CONSOLE_UART_RELEASE_RESET()            __HAL_RCC_USART1_RELEASE_RESET()
#define CONSOLE_UART_TX_PIN                     GPIO_PIN_6
#define CONSOLE_UART_TX_GPIO_PORT               GPIOB
#define CONSOLE_UART_TX_AF                      GPIO_AF7_USART1
#define CONSOLE_UART_RX_PIN                     GPIO_PIN_7
#define CONSOLE_UART_RX_GPIO_PORT               GPIOB
#define CONSOLE_UART_RX_AF                      GPIO_AF7_USART1
#define CONSOLE_UART_IRQn                       USART1_IRQn
/* TODO: 扩展串口(控制台)定义 */

/* I2C个数 MPU9250+功能板 */
#define I2C_NUMS                                (1)
/* IMU I2C定义 */
#define IMU_I2C                                 I2C1
#define IMU_I2C_CLOCK_ENABLE()                  __I2C1_CLK_ENABLE()
#define IMU_I2C_FORCE_RESET()                   __I2C1_FORCE_RESET()
#define IMU_I2C_RELEASE_RESET()                 __I2C1_RELEASE_RESET()
#define IMU_I2C_SDA_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE()
#define IMU_I2C_SCL_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE() 
#define IMU_I2C_SDA_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE()
#define IMU_I2C_SCL_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE() 
#define IMU_I2C_SCL_PIN                         GPIO_PIN_8
#define IMU_I2C_SCL_GPIO_PORT                   GPIOB
#define IMU_I2C_SCL_SDA_AF                      GPIO_AF4_I2C1
#define IMU_I2C_SDA_PIN                         GPIO_PIN_9
#define IMU_I2C_SDA_GPIO_PORT                   GPIOB
#define IMU_I2C_EV_IRQn                         I2C1_EV_IRQn
#define IMU_I2C_ER_IRQn                         I2C1_ER_IRQn
/* MPU9250中断 */
#define IMU_INT_PIN                             GPIO_PIN_13
#define IMU_INT_GPIO_PORT                       GPIOC
#define IMU_INT_CLK_ENABLE()                    __GPIOC_CLK_ENABLE()
#define IMU_INT_EXTI                            EXTI15_10_IRQn

/* TODO:功能板 I2C定义 */

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
void clock_init(void);

#endif /* _STM32F4XX_HAL_MSP_H_ */

