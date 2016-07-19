/******************************************************************************
 *
 * 文件名  ： stm32f4xx_hal_conf.h
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20150116
 * 版本号  ： v1.0
 * 文件描述： STM32 HAL 配置文件
 * 版权说明： Copyright (c) 2000-2020 GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _HAL_STM32F4XX_HAL_CONF_H_
#define _HAL_STM32F4XX_HAL_CONF_H_

/************************************ 头文件 ***********************************/

/************************************ 宏定义 ***********************************/
/*********************************** 模块选择 **********************************/
/* 定义需要使用的模块 */
#define HAL_MODULE_ENABLED
/* #define HAL_ADC_MODULE_ENABLED       */
/* #define HAL_CAN_MODULE_ENABLED       */
#define HAL_CRC_MODULE_ENABLED
/* #define HAL_CRYP_MODULE_ENABLED      */
/* #define HAL_DAC_MODULE_ENABLED       */
/* #define HAL_DCMI_MODULE_ENABLED      */
#define HAL_DMA_MODULE_ENABLED
/* #define HAL_DMA2D_MODULE_ENABLED     */
/* #define HAL_ETH_MODULE_ENABLED       */
#define HAL_FLASH_MODULE_ENABLED
/* #define HAL_NAND_MODULE_ENABLED      */
/* #define HAL_NOR_MODULE_ENABLED       */
/* #define HAL_PCCARD_MODULE_ENABLED    */
/* #define HAL_SRAM_MODULE_ENABLED      */
/* #define HAL_SDRAM_MODULE_ENABLED     */
/* #define HAL_HASH_MODULE_ENABLED      */
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
/* #define HAL_I2S_MODULE_ENABLED       */
/* #define HAL_IWDG_MODULE_ENABLED      */
/* #define HAL_LTDC_MODULE_ENABLED      */
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
/* #define HAL_RNG_MODULE_ENABLED       */
/* #define HAL_RTC_MODULE_ENABLED       */
/* #define HAL_SAI_MODULE_ENABLED       */
/* #define HAL_SD_MODULE_ENABLED        */
/* #define HAL_SPI_MODULE_ENABLED       */
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
/* #define HAL_USART_MODULE_ENABLED     */
/* #define HAL_IRDA_MODULE_ENABLED      */
/* #define HAL_SMARTCARD_MODULE_ENABLED */
/* #define HAL_WWDG_MODULE_ENABLED      */
#define HAL_CORTEX_MODULE_ENABLED
/* #define HAL_PCD_MODULE_ENABLED       */
/*#define HAL_HCD_MODULE_ENABLED        */

/********************************** HSE/HSI值 **********************************/
/* 外部晶体(HSE)频率(Hz) */
/* 需要对应晶体振荡器的频率 */
#if !defined  (HSE_VALUE)
  #define HSE_VALUE    ((uint32_t)8000000)
#endif /* HSE_VALUE */

/* 晶体启振时间(ms) */
#if !defined  (HSE_STARTUP_TIMEOUT)
    #define HSE_STARTUP_TIMEOUT    ((uint32_t)5000)
#endif /* HSE_STARTUP_TIMEOUT */

/* 内部晶体(HSI)频率(Hz) */
#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)16000000)
#endif /* HSI_VALUE */

/* 内部晶体(LSI)频率(Hz) */
#if !defined  (LSI_VALUE)
 #define LSI_VALUE  ((uint32_t)32000)
#endif /* LSI_VALUE */

/* 外部晶体(LSE)频率(Hz) */
#if !defined  (LSE_VALUE)
 #define LSE_VALUE  ((uint32_t)32768)
#endif /* LSE_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    ((uint32_t)5000U)   /*!< Time out for LSE start up, in ms */
#endif /* LSE_STARTUP_TIMEOUT */

/************************************ HAL配置 **********************************/
#define  VDD_VALUE                    ((uint32_t)3300) /* VDD电压(mv) */
#define  USE_RTOS                     0
#define  PREFETCH_ENABLE              1
#define  INSTRUCTION_CACHE_ENABLE     1
#define  DATA_CACHE_ENABLE            1

/* Fault 0 > (传感器)I2C DMA > 外设 > SysTick > PendSV */
#define  MEM_INT_PRIORITY             ((uint32_t)0) /* Memory Management 中断优先级 */
#define  BUS_INT_PRIORITY             ((uint32_t)0) /* Bus Fault 中断优先级 */
#define  USAGE_INT_PRIORITY           ((uint32_t)0) /* Usage Fault 中断优先级 */
#define  PER_SI_INT_PRIORITY          ((uint32_t)12) /* I2C DMA读取优先级需要较高 否则同步会故障 */
#define  PER_INT_PRIORITY             ((uint32_t)13) /* UART|I2C|LED|PWM等外设的中断的优先级 */
#define  TICK_INT_PRIORITY            ((uint32_t)14) /* SysTick 中断优先级较低 */
/* 最低优先级15预留给PendSV做任务切换 */

/* include头文件 */
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f4xx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f4xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f4xx_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f4xx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

#ifdef HAL_ADC_MODULE_ENABLED
  #include "stm32f4xx_hal_adc.h"
#endif /* HAL_ADC_MODULE_ENABLED */

#ifdef HAL_CAN_MODULE_ENABLED
  #include "stm32f4xx_hal_can.h"
#endif /* HAL_CAN_MODULE_ENABLED */

#ifdef HAL_CRC_MODULE_ENABLED
  #include "stm32f4xx_hal_crc.h"
#endif /* HAL_CRC_MODULE_ENABLED */

#ifdef HAL_CRYP_MODULE_ENABLED
  #include "stm32f4xx_hal_cryp.h"
#endif /* HAL_CRYP_MODULE_ENABLED */

#ifdef HAL_DMA2D_MODULE_ENABLED
  #include "stm32f4xx_hal_dma2d.h"
#endif /* HAL_DMA2D_MODULE_ENABLED */

#ifdef HAL_DAC_MODULE_ENABLED
  #include "stm32f4xx_hal_dac.h"
#endif /* HAL_DAC_MODULE_ENABLED */

#ifdef HAL_DCMI_MODULE_ENABLED
  #include "stm32f4xx_hal_dcmi.h"
#endif /* HAL_DCMI_MODULE_ENABLED */

#ifdef HAL_ETH_MODULE_ENABLED
  #include "stm32f4xx_hal_eth.h"
#endif /* HAL_ETH_MODULE_ENABLED */

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f4xx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */

#ifdef HAL_SRAM_MODULE_ENABLED
  #include "stm32f4xx_hal_sram.h"
#endif /* HAL_SRAM_MODULE_ENABLED */

#ifdef HAL_NOR_MODULE_ENABLED
  #include "stm32f4xx_hal_nor.h"
#endif /* HAL_NOR_MODULE_ENABLED */

#ifdef HAL_NAND_MODULE_ENABLED
  #include "stm32f4xx_hal_nand.h"
#endif /* HAL_NAND_MODULE_ENABLED */

#ifdef HAL_PCCARD_MODULE_ENABLED
  #include "stm32f4xx_hal_pccard.h"
#endif /* HAL_PCCARD_MODULE_ENABLED */

#ifdef HAL_SDRAM_MODULE_ENABLED
  #include "stm32f4xx_hal_sdram.h"
#endif /* HAL_SDRAM_MODULE_ENABLED */

#ifdef HAL_HASH_MODULE_ENABLED
 #include "stm32f4xx_hal_hash.h"
#endif /* HAL_HASH_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
 #include "stm32f4xx_hal_i2c.h"
#endif /* HAL_I2C_MODULE_ENABLED */

#ifdef HAL_I2S_MODULE_ENABLED
 #include "stm32f4xx_hal_i2s.h"
#endif /* HAL_I2S_MODULE_ENABLED */

#ifdef HAL_IWDG_MODULE_ENABLED
 #include "stm32f4xx_hal_iwdg.h"
#endif /* HAL_IWDG_MODULE_ENABLED */

#ifdef HAL_LTDC_MODULE_ENABLED
 #include "stm32f4xx_hal_ltdc.h"
#endif /* HAL_LTDC_MODULE_ENABLED */

#ifdef HAL_PWR_MODULE_ENABLED
 #include "stm32f4xx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

#ifdef HAL_RNG_MODULE_ENABLED
 #include "stm32f4xx_hal_rng.h"
#endif /* HAL_RNG_MODULE_ENABLED */

#ifdef HAL_RTC_MODULE_ENABLED
 #include "stm32f4xx_hal_rtc.h"
#endif /* HAL_RTC_MODULE_ENABLED */

#ifdef HAL_SAI_MODULE_ENABLED
 #include "stm32f4xx_hal_sai.h"
#endif /* HAL_SAI_MODULE_ENABLED */

#ifdef HAL_SD_MODULE_ENABLED
 #include "stm32f4xx_hal_sd.h"
#endif /* HAL_SD_MODULE_ENABLED */

#ifdef HAL_SPI_MODULE_ENABLED
 #include "stm32f4xx_hal_spi.h"
#endif /* HAL_SPI_MODULE_ENABLED */

#ifdef HAL_TIM_MODULE_ENABLED
 #include "stm32f4xx_hal_tim.h"
#endif /* HAL_TIM_MODULE_ENABLED */

#ifdef HAL_UART_MODULE_ENABLED
 #include "stm32f4xx_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */

#ifdef HAL_USART_MODULE_ENABLED
 #include "stm32f4xx_hal_usart.h"
#endif /* HAL_USART_MODULE_ENABLED */

#ifdef HAL_IRDA_MODULE_ENABLED
 #include "stm32f4xx_hal_irda.h"
#endif /* HAL_IRDA_MODULE_ENABLED */

#ifdef HAL_SMARTCARD_MODULE_ENABLED
 #include "stm32f4xx_hal_smartcard.h"
#endif /* HAL_SMARTCARD_MODULE_ENABLED */

#ifdef HAL_WWDG_MODULE_ENABLED
 #include "stm32f4xx_hal_wwdg.h"
#endif /* HAL_WWDG_MODULE_ENABLED */

#ifdef HAL_PCD_MODULE_ENABLED
 #include "stm32f4xx_hal_pcd.h"
#endif /* HAL_PCD_MODULE_ENABLED */

#ifdef HAL_HCD_MODULE_ENABLED
 #include "stm32f4xx_hal_hcd.h"
#endif /* HAL_HCD_MODULE_ENABLED */

/* 向量表的偏移 必须为0x200 的倍数 */
#define VECT_TAB_OFFSET  0x00

/*********************************** 断言配置 **********************************/
#define USE_FULL_ASSERT                         (1)
/*******************************************************************************
 *
 * 函数名  : assert_param
 * 负责人  : 彭鹏
 * 创建日期：20150321
 * 函数功能: 用于函数做参数检查 HAL源码已经使用
 *
 * 输入参数: expr
 *           - true  啥也不干
 *           - flase 该函数调用 assert_failed 报告源码文件及行数
 *
 * 输出参数: 无
 *
 * 返回值:   无
 *
 * 调用关系: 无
 * 其 它:    无
 *
 *
 ******************************************************************************/
#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

/*********************************** 类型定义 **********************************/

/*--------------------------------- 接口声明区 --------------------------------*/

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/

#endif /* #ifdef _HAL_STM32F4XX_HAL_CONF_H_ */

