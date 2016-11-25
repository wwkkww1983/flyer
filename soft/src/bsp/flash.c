/******************************************************************************
 *
 * 文件名  ： flash.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20161125 
 * 版本号  ： 1.0
 * 文件描述： flash模块
 * 版权说明： Copyright (c) GNU
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/

/*---------------------------------- 预处理区 ---------------------------------*/
//#pragma  diag_suppress 870

/************************************ 头文件 ***********************************/
#include "config.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "flash.h"
#include "debug.h"

/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/

/********************************** 函数声明区 *********************************/
#if 0
static void flash_test(void);
#endif

/********************************** 函数实现区 *********************************/
void flash_init(void)
{

#if 0
    flash_test();
#endif

} 

#if 0
/* flash 测试 */
static void flash_test(void)
{
    uint32_T data = 0x00000000;
    uint32_T *ptr_data = NULL;

    /* step1: 回读确保为全0xFF,避免与程序段冲突 */
    for(ptr_data = FLASH_USER_START; ptr_data < FLASH_USER_END; ptr_data++) 
    {
        data = *ptr_data;

        if(0xA5A5A5A5 == data)
        {
            debug_log("已经做过写入测试,验证成功.\r\n");
            return;
        }

        if(0xFFFFFFFF != data)
        {
            ERR_STR("flash测试step1失败.\r\n");
        }

        ptr_data++;
    }

    /* step2: 全部写入0xA5A5A5A5 */
    data = 0xA5A5A5A5;
    HAL_FLASH_Unlock();
    for(ptr_data = FLASH_USER_START; ptr_data < FLASH_USER_END; ptr_data++) 
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)ptr_data, data);
        ptr_data++;
    }
    HAL_FLASH_Lock();

    /* step2: 回读确保为全0xA5A5A5A5,避免与程序段冲突 */
    for(ptr_data = FLASH_USER_START; ptr_data < FLASH_USER_END; ptr_data++) 
    {
        data = *ptr_data;
        if(0xA5A5A5A5 != data)
        {
            ERR_STR("flash测试step3失败.\r\n");
        }

        ptr_data++;
    }

    debug_log("flash测试通过.\r\n");
}
#endif

