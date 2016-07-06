;******************************************************************************
;*
;* 文件名  ： stm32f429xx.s
;* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
;* 创建日期： 20150614
;* 版本号  ： v1.0
;* 文件描述： stm32f429xx芯片启动汇编(for MDK-ARM)
;*            它主要完成
;*            - 设置向量表
;*            - 设置初始SP
;*            - 设置初始PC == Reset_Handler
;*            - 跳转到C运行时库的__main函数,该函数会调用main
;*            - 设置系统堆栈
;*
;*            复位之后Cortex-M4进入Tread模式
;*            特权级别(Privileged),SP使用MSP
;*
;* 版权说明： Copyright (c) 2000-2020 GNU
;* 其    他： 无
;* 修改日志： 无
;*
;*******************************************************************************/

; 系统栈(MSP) 2kB
Stack_Size      EQU     0x00000800

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

; 堆 2kB
; 对比.map文件中
Heap_Size       EQU     0x00000800

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

; 复位后的向量表
; 位于0x00000000
                AREA   RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp            ; Top of Stack
                DCD     Reset_Handler           ; Reset Handler
                DCD     NMI_Handler             ; NMI Handler
                DCD     HardFault_Handler       ; Hard Fault Handler
                DCD     MemManage_Handler       ; MPU Fault Handler
                DCD     BusFault_Handler        ; Bus Fault Handler
                DCD     UsageFault_Handler      ; Usage Fault Handler
                DCD     0                       ; Reserved
                DCD     0                       ; Reserved
                DCD     0                       ; Reserved
                DCD     0                       ; Reserved
                DCD     0                       ; SVCall Handler
                DCD     DebugMon_Handler        ; Debug Monitor Handler
                DCD     0                       ; Reserved
                DCD     0                       ; PendSV Handler
                DCD     SysTick_Handler         ; SysTick Handler
                DCD     0                       ; Window WatchDog
                DCD     0                       ; PVD through EXTI Line detection
                DCD     0                       ; Tamper and TimeStamps through the EXTI line
                DCD     0                       ; RTC Wakeup through the EXTI line
                DCD     0                       ; FLASH
                DCD     0                       ; RCC
                DCD     0                       ; EXTI Line0
                DCD     0                       ; EXTI Line1
                DCD     0                       ; EXTI Line2
                DCD     0                       ; EXTI Line3
                DCD     0                       ; EXTI Line4
                DCD     0                       ; DMA1 Stream 0
                DCD     0                       ; DMA1 Stream 1
                DCD     0                       ; DMA1 Stream 2
                DCD     0                       ; DMA1 Stream 3
                DCD     0                       ; DMA1 Stream 4
                DCD     0                       ; DMA1 Stream 5
                DCD     0                       ; DMA1 Stream 6
                DCD     0                       ; ADC1, ADC2 and ADC3s
                DCD     0                       ; CAN1 TX
                DCD     0                       ; CAN1 RX0
                DCD     0                       ; CAN1 RX1
                DCD     0                       ; CAN1 SCE
                DCD     0                       ; External Line[9:5]s
                DCD     0                       ; TIM1 Break and TIM9
                DCD     0                       ; TIM1 Update and TIM10
                DCD     0                       ; TIM1 Trigger and Commutation and TIM11
                DCD     0                       ; TIM1 Capture Compare
                DCD     0                       ; TIM2
                DCD     0                       ; TIM3
                DCD     0                       ; TIM4
                DCD     0                       ; I2C1 Event
                DCD     0                       ; I2C1 Error
                DCD     0                       ; I2C2 Event
                DCD     0                       ; I2C2 Error
                DCD     0                       ; SPI1
                DCD     0                       ; SPI2
                DCD     USART1_IRQHandler       ; USART1
                DCD     0                       ; USART2
                DCD     0                       ; USART3
                DCD     EXTI15_10_IRQHandler    ; External Line[15:10]s
                DCD     0                       ; RTC Alarm (A and B) through EXTI Line
                DCD     0                       ; USB OTG FS Wakeup through EXTI line
                DCD     0                       ; TIM8 Break and TIM12
                DCD     0                       ; TIM8 Update and TIM13
                DCD     0                       ; TIM8 Trigger and Commutation and TIM14
                DCD     0                       ; TIM8 Capture Compare
                DCD     0                       ; DMA1 Stream7
                DCD     0                       ; FMC
                DCD     0                       ; SDIO
                DCD     0                       ; TIM5
                DCD     0                       ; SPI3
                DCD     0                       ; UART4
                DCD     0                       ; UART5
                DCD     0                       ; TIM6 and DAC1&2 underrun errors
                DCD     0                       ; TIM7
                DCD     0                       ; DMA2 Stream 0
                DCD     0                       ; DMA2 Stream 1
                DCD     0                       ; DMA2 Stream 2
                DCD     0                       ; DMA2 Stream 3
                DCD     0                       ; DMA2 Stream 4
                DCD     0                       ; Ethernet
                DCD     0                       ; Ethernet Wakeup through EXTI line
                DCD     0                       ; CAN2 TX
                DCD     0                       ; CAN2 RX0
                DCD     0                       ; CAN2 RX1
                DCD     0                       ; CAN2 SCE
                DCD     0                       ; USB OTG FS
                DCD     0                       ; DMA2 Stream 5
                DCD     DMA2_Stream6_IRQHandler ; DMA2 Stream 6
                DCD     DMA2_Stream7_IRQHandler ; DMA2 Stream 7
                DCD     USART6_IRQHandler       ; USART6
                DCD     0                       ; I2C3 event
                DCD     0                       ; I2C3 error
                DCD     0                       ; USB OTG HS End Point 1 Out
                DCD     0                       ; USB OTG HS End Point 1 In
                DCD     0                       ; USB OTG HS Wakeup through EXTI
                DCD     0                       ; USB OTG HS
                DCD     0                       ; DCMI
                DCD     0                       ; Reserved
                DCD     0                       ; Hash and Rng
                DCD     0                       ; FPU
                DCD     0                       ; UART7
                DCD     0                       ; UART8
                DCD     0                       ; SPI4
                DCD     0                       ; SPI5
                DCD     0                       ; SPI6
                DCD     0                       ; SAI1
                DCD     0                       ; LTDC
                DCD     0                       ; LTDC error
                DCD     0                       ; DMA2D
__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

; 复位句柄
Reset_Handler    PROC
                 EXPORT  Reset_Handler
                 IMPORT  SystemInit
                 IMPORT  __main

                 LDR     R0, =SystemInit
                 BLX     R0
                 LDR     R0, =__main
                 BX      R0
                 ENDP

; 未使用的异常 死循环
; TODO: 加入异常处理
NMI_Handler     PROC
                EXPORT  NMI_Handler
                B       .
                ENDP

HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler
                B       .
                ENDP

MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler
                B       .
                ENDP

BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler
                B       .
                ENDP

UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler
                B       .
                ENDP

DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler
                B       .
                ENDP

; 系统心跳句柄 C代码有重定义
SysTick_Handler PROC
                EXPORT  SysTick_Handler  [WEAK]
                B       .
                ENDP

; 控制台串口中断 C代码有重定义
USART1_IRQHandler PROC
                EXPORT  USART1_IRQHandler [WEAK]
                B       .
                ENDP
DMA2_Stream7_IRQHandler PROC
                EXPORT  DMA2_Stream7_IRQHandler [WEAK]
                B       .
                ENDP


; ESP8266串口中断 C代码有重定义
USART6_IRQHandler PROC
                EXPORT  USART6_IRQHandler [WEAK]
                B       .
                ENDP
DMA2_Stream6_IRQHandler PROC
                EXPORT  DMA2_Stream6_IRQHandler [WEAK]
                B       .
                ENDP

; External Line[15:10]中断 C代码有重定义
EXTI15_10_IRQHandler PROC
                EXPORT  EXTI15_10_IRQHandler [WEAK]
                B       .
                ENDP

; IMU I2C读取代码
;I2C1_EV_IRQHandler PROC
                ;EXPORT  I2C1_EV_IRQHandler [WEAK]
                ;B       .
                ;ENDP

                ALIGN

;*******************************************************************************
; Keil工具链完成 堆栈初始化
;*******************************************************************************
                 IF      :DEF:__MICROLIB

                 EXPORT  __initial_sp
                 EXPORT  __heap_base
                 EXPORT  __heap_limit

                 ELSE

                 IMPORT  __use_two_region_memory
                 EXPORT  __user_initial_stackheap

__user_initial_stackheap
                 LDR     R0, =  Heap_Mem
                 LDR     R1, =(Stack_Mem + Stack_Size)
                 LDR     R2, = (Heap_Mem +  Heap_Size)
                 LDR     R3, = Stack_Mem
                 BX      LR

                 ALIGN

                 ENDIF

                 END

