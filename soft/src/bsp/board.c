/******************************************************************************
 *
 * 文件名  ： board.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20150615
 * 版本号  ： 1.0
 * 文件描述： stm32f4 cube hal 初始化回调
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"
#include "board.h"

#include "console.h"
#include "esp8266.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数声明区 *********************************/

/********************************** 函数实现区 *********************************/
/*******************************************************************************
*
* 函数名  : HAL_MspInit
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: stm32f4 hal初始化回调
*           HAL_Init 中统一调用
*           配置复用的管腿
* 输入参数: 无
* 输出参数: 无
* 返回值  : 无
*
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void HAL_MspInit()
{  
    /* 两个hdma变量运行时生命域 必须加入static修饰 */
    static DMA_HandleTypeDef console_hdma_tx;
    static DMA_HandleTypeDef esp8266_hdma_tx;

    GPIO_InitTypeDef GPIO_InitStruct;

    /************************ 控制台使用的串口初始化 ************************/
    CONSOLE_UART_TX_GPIO_CLK_ENABLE();
    CONSOLE_UART_RX_GPIO_CLK_ENABLE();
    CONSOLE_UART_CLK_ENABLE(); 
    CONSOLE_DMA_CLK_ENABLE();   
    /* TX */
    GPIO_InitStruct.Mode                = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull                = GPIO_NOPULL;
    GPIO_InitStruct.Speed               = GPIO_SPEED_FAST;
    GPIO_InitStruct.Pin                 = CONSOLE_UART_TX_PIN;
    GPIO_InitStruct.Alternate           = CONSOLE_UART_TX_AF;
    HAL_GPIO_Init(CONSOLE_UART_TX_GPIO_PORT, &GPIO_InitStruct);
    /* RX */
    GPIO_InitStruct.Mode                = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull                = GPIO_NOPULL;
    GPIO_InitStruct.Speed               = GPIO_SPEED_FAST;
    GPIO_InitStruct.Pin                 = CONSOLE_UART_RX_PIN;
    GPIO_InitStruct.Alternate           = CONSOLE_UART_RX_AF;
    HAL_GPIO_Init(CONSOLE_UART_RX_GPIO_PORT, &GPIO_InitStruct); 
    /* 配置DMA(仅TX) */
    console_hdma_tx.Instance                    = CONSOLE_UART_TX_DMA_STREAM; 
    console_hdma_tx.Init.Channel                = CONSOLE_UART_TX_DMA_CHANNEL;
    console_hdma_tx.Init.Direction              = DMA_MEMORY_TO_PERIPH;
    console_hdma_tx.Init.PeriphInc              = DMA_PINC_DISABLE;
    console_hdma_tx.Init.MemInc                 = DMA_MINC_ENABLE;
    console_hdma_tx.Init.PeriphDataAlignment    = DMA_PDATAALIGN_BYTE;
    console_hdma_tx.Init.MemDataAlignment       = DMA_MDATAALIGN_BYTE;
    console_hdma_tx.Init.Mode                   = DMA_NORMAL;
    console_hdma_tx.Init.Priority               = DMA_PRIORITY_LOW;
    console_hdma_tx.Init.FIFOMode               = DMA_FIFOMODE_DISABLE;
    console_hdma_tx.Init.FIFOThreshold          = DMA_FIFO_THRESHOLD_FULL;
    console_hdma_tx.Init.MemBurst               = DMA_MBURST_INC4;
    console_hdma_tx.Init.PeriphBurst            = DMA_PBURST_INC4; 
    HAL_DMA_Init(&console_hdma_tx);   
    /* 关联DMA与UART */
    __HAL_LINKDMA(&g_console.handle, hdmatx, console_hdma_tx); 
    /* INT */
    HAL_NVIC_SetPriority(CONSOLE_UART_IRQn, PER_INT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(CONSOLE_UART_IRQn); 
    HAL_NVIC_SetPriority(CONSOLE_UART_DMA_TX_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(CONSOLE_UART_DMA_TX_IRQn);

    /************************* ESP8266串口初始化 ****************************/
    ESP8266_UART_TX_GPIO_CLK_ENABLE();
    ESP8266_UART_RX_GPIO_CLK_ENABLE();
    ESP8266_UART_CLK_ENABLE(); 
    ESP8266_DMA_CLK_ENABLE();   
    /* TX */
    GPIO_InitStruct.Mode                = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull                = GPIO_NOPULL;
    GPIO_InitStruct.Speed               = GPIO_SPEED_FAST;
    GPIO_InitStruct.Pin                 = ESP8266_UART_TX_PIN;
    GPIO_InitStruct.Alternate           = ESP8266_UART_TX_AF;
    HAL_GPIO_Init(ESP8266_UART_TX_GPIO_PORT, &GPIO_InitStruct);
    /* RX */
    GPIO_InitStruct.Mode                = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull                = GPIO_NOPULL;
    GPIO_InitStruct.Speed               = GPIO_SPEED_FAST;
    GPIO_InitStruct.Pin                 = ESP8266_UART_RX_PIN;
    GPIO_InitStruct.Alternate           = ESP8266_UART_RX_AF;
    HAL_GPIO_Init(ESP8266_UART_RX_GPIO_PORT, &GPIO_InitStruct); 
    /* 配置DMA(仅TX) */
    esp8266_hdma_tx.Instance                    = ESP8266_UART_TX_DMA_STREAM; 
    esp8266_hdma_tx.Init.Channel                = ESP8266_UART_TX_DMA_CHANNEL;
    esp8266_hdma_tx.Init.Direction              = DMA_MEMORY_TO_PERIPH;
    esp8266_hdma_tx.Init.PeriphInc              = DMA_PINC_DISABLE;
    esp8266_hdma_tx.Init.MemInc                 = DMA_MINC_ENABLE;
    esp8266_hdma_tx.Init.PeriphDataAlignment    = DMA_PDATAALIGN_BYTE;
    esp8266_hdma_tx.Init.MemDataAlignment       = DMA_MDATAALIGN_BYTE;
    esp8266_hdma_tx.Init.Mode                   = DMA_NORMAL;
    esp8266_hdma_tx.Init.Priority               = DMA_PRIORITY_LOW;
    esp8266_hdma_tx.Init.FIFOMode               = DMA_FIFOMODE_DISABLE;
    esp8266_hdma_tx.Init.FIFOThreshold          = DMA_FIFO_THRESHOLD_FULL;
    esp8266_hdma_tx.Init.MemBurst               = DMA_MBURST_INC4;
    esp8266_hdma_tx.Init.PeriphBurst            = DMA_PBURST_INC4;
    HAL_DMA_Init(&esp8266_hdma_tx);   
    /* 关联DMA与UART */
    __HAL_LINKDMA(&g_esp8266.handle, hdmatx, esp8266_hdma_tx); 
    /* INT */
    HAL_NVIC_SetPriority(ESP8266_UART_IRQn, PER_INT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(ESP8266_UART_IRQn); 
    HAL_NVIC_SetPriority(ESP8266_UART_DMA_TX_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(ESP8266_UART_DMA_TX_IRQn);
}

#if 0
/*******************************************************************************
*
* 函数名  : HAL_TIM_PWM_MspInit
* 负责人  : 彭鹏
* 创建日期: 20160126
* 函数功能: stm32f4 hal timer pwm初始化回调
*
* 输入参数: stm32 hal timer句柄
* 输出参数: 无
* 返回值  : 无
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef   GPIO_InitStruct;
   
    TIMx_CLK_ENABLE(); 
    TIMx_CHANNEL1_PORT_CLK_ENABLE();
    TIMx_CHANNEL2_PORT_CLK_ENABLE();
    TIMx_CHANNEL3_PORT_CLK_ENABLE();
    TIMx_CHANNEL4_PORT_CLK_ENABLE();
    
    /* 配置PWM端口 */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL1;
    GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL1;
    HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL1, &GPIO_InitStruct);

    GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL2;
    GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL2;
    HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL2, &GPIO_InitStruct);

    GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL3;
    GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL3;
    HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL3, &GPIO_InitStruct);

    GPIO_InitStruct.Alternate = TIMx_GPIO_AF_CHANNEL4;
    GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL4;
    HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL4, &GPIO_InitStruct);
}

/*******************************************************************************
*
* 函数名  : HAL_TIM_PWM_MspDeInit
* 负责人  : 彭鹏
* 创建日期: 20160126
* 函数功能: stm32f4 hal timer pwm解初始化回调
*
* 输入参数: stm32 hal timer句柄
* 输出参数: 无
* 返回值  : 无
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim)
{
}


/*******************************************************************************
*
* 函数名  : HAL_UART_MspDeInit
* 负责人  : 彭鹏
* 创建日期: 20150615
* 函数功能: stm32f4 hal uart解初始化回调
*
* 输入参数: stm32 hal uart句柄
*
* 输出参数: 无
*
* 返回值  : 无
*
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
}

/*******************************************************************************
*
* 函数名  : HAL_I2C_MspInit
* 负责人  : 彭鹏
* 创建日期: 20151113
* 函数功能: stm32f4 hal i2c初始化回调
*
* 输入参数: stm32 hal i2c句柄
* 输出参数: 无
*
* 返回值  : 无
*
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef  GPIO_InitStruct; 

    /************************** 其他I2C3总线初始化 **************************/
    IMU_I2C_SDA_GPIO_CLK_ENABLE();
    IMU_I2C_SCL_GPIO_CLK_ENABLE();

    /* 配置管脚 */
    GPIO_InitStruct.Pin       = IMU_I2C_SCL_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = IMU_I2C_SCL_SDA_AF;
    HAL_GPIO_Init(IMU_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = IMU_I2C_SDA_PIN;
    HAL_GPIO_Init(IMU_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);

    /* 使能时钟 */
    IMU_I2C_CLOCK_ENABLE();
    IMU_I2C_FORCE_RESET();
    IMU_I2C_RELEASE_RESET(); 
    
    /* 设置I2C中断优先级 */
    HAL_NVIC_SetPriority(IMU_I2C_EV_IRQn, PER_INT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(IMU_I2C_EV_IRQn);
    HAL_NVIC_SetPriority(IMU_I2C_ER_IRQn, PER_INT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(IMU_I2C_ER_IRQn); 
    
    /************************** 其他I2C总线初始化 ***************************/
}

/*******************************************************************************
*
* 函数名  : HAL_I2C_MspDeInit
* 负责人  : 彭鹏
* 创建日期: 20151113
* 函数功能: stm32f4 hal i2c解初始化回调
*
* 输入参数: stm32 hal i2c句柄
* 输出参数: 无
*
* 返回值  : 无
*
* 调用关系: 无
* 其 它   : TODO: 实现stm32f4 hal 层解初始化
*
******************************************************************************/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
}
#endif

/*******************************************************************************
*
* 函数名  : clock_init
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: 时钟配置
*           系统时钟配置如下: 
*           System Clock source                     = PLL (HSE)
*           SYSCLK(Hz)                              = 84000000(84Mhz)
*           HCLK(Hz)                                = 84000000(84Mhz)
*           AHB Prescaler                           = 1
*           APB1 Prescaler                          = 2
*           APB2 Prescaler                          = 1
*           HSE Frequency(Hz)                       = 8000000(8Mhz)
*           PLL_M                                   = 8
*           PLL_N                                   = 336
*           PLL_P                                   = 4
*           PLL_Q                                   = 7
*           VDD(V)                                  = 3.3
*           Main regulator output voltage           = Scale2 mode
*           Flash Latency(WS)                       = 2
*
* 输入参数: 无
* 输出参数: 无
* 返回值  : 无
* 调用关系: 无
* 其 它   : 84MHz
*
******************************************************************************/
void clock_init(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* 使能PWR CLK模块 */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* 10 = VOS[1:0](位于PWR_CR寄存器),最高频率84Mhz */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  /* PLL使用外部晶体,并且激活PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
      while(1);
  }
 

  /* 选择PLL作为系统主时钟并且配置HCLK,PCLK1,PCLK2分频数 */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
      while(1);
  }
}

