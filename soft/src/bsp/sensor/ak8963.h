/********************************************************************************
*
* 文件名  ： ak8963.h
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20150614
* 版本号  ： v1.0
* 文件描述： ak8963驱动头文件
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/
#ifndef _AK8963_H_
#define _AK8963_H_

/************************************ 头文件 ***********************************/
#include "typedef.h"
#include "config.h"

/************************************ 宏定义 ***********************************/

/*********************************** 板级定义 **********************************/
#define AK8963_DEV_ADDR                     (0x18)
#define AK8963_WIA_REG_ADDR                 (0x00)
#define AK8963_INFO_REG_ADDR                (0x01)
#define AK8963_ST1_REG_ADDR                 (0x02)
#define AK8963_HXL_REG_ADDR                 (0x03)
#define AK8963_HXH_REG_ADDR                 (0x04)
#define AK8963_HYL_REG_ADDR                 (0x05)
#define AK8963_HYH_REG_ADDR                 (0x06)
#define AK8963_HZL_REG_ADDR                 (0x07)
#define AK8963_HZH_REG_ADDR                 (0x08)
#define AK8963_ST2_REG_ADDR                 (0x09)
#define AK8963_CNTL1_REG_ADDR               (0x0A)
#define AK8963_CNTL2_REG_ADDR               (0x0B)
#define AK8963_ASTC_REG_ADDR                (0x0C)
#define AK8963_ASAX_REG_ADDR                (0x10)
#define AK8963_ASAY_REG_ADDR                (0x11)
#define AK8963_ASAZ_REG_ADDR                (0x12)
#define AK8963_WIA_REG_VAL                  (0x48)
#define AK8963_ST1_DRDY_BIT                 (0x01)
#define AK8963_ST1_DOR_BIT                  (0x02)
#define AK8963_ST2_HOFL_BIT                 (0x08)
#define AK8963_14BITS                       (0x00)
#define AK8963_16BITS                       (0x10)
#define AK8963_MODE_PD                      (0x00)
#define AK8963_MODE_S                       (0x01)
#define AK8963_MODE_C1                      (0x02)
#define AK8963_MODE_C2                      (0x06)
#define AK8963_MODE_ET                      (0x04)
#define AK8963_MODE_TEST                    (0x08)
#define AK8963_MODE_FUSE                    (0x0F)
#define AK8963_POWER_DOWN                   (AK8963_16BITS | AK8963_MODE_PD)
#define AK8963_FUSE_ACCESS                  (AK8963_16BITS | AK8963_MODE_FUSE)
#define AK8963_16BIT_STEP                   (0.15f)
#define AK8963_DATA_FIRST_ADDR              (AK8963_ST1_REG_ADDR)
#define AK8963_DATA_LENGTH                  (AK8963_ST2_REG_ADDR - AK8963_ST1_REG_ADDR + 1)

/*--------------------------------- 接口声明区 --------------------------------*/
typedef struct{
    uint8_T st1;
    uint8_T st2;
    f32_T val[3];
}ak8963_val_T;

/*********************************** 全局变量 **********************************/

/*********************************** 接口函数 **********************************/
/* 初始化 */
void ak8963_init(void);
int32_T ak8963_read(uint8_T *buf);
int32_T ak8963_parse(ak8963_val_T *val, const uint8_T *buf);
void ak8963_test(void);

#endif /* _AK8963_H_ */

