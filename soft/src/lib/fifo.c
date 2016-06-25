/********************************************************************************
*
* 文件名  ： fifo.c
* 负责人  ： 彭鹏(pengpeng@fiberhome.com)
* 创建日期： 20160624
* 版本号  ： v1.0
* 文件描述： 先入先出数据结构
* 版权说明： Copyright (c) 2000-2020 GNU
* 其 他   ： 无
* 修改日志： 无
*
********************************************************************************/
/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include "stdio.h"
#include "stdlib.h"

#include "config.h"
#include "typedef.h"
#include "fifo.h"

/************************************ 宏定义 ***********************************/

/*********************************** 类型定义 **********************************/

/*--------------------------------- 接口声明区 --------------------------------*/
static int32_T fifo_size(const fifo_T *fifo);
static int32_T fifo_space(const fifo_T *fifo);
static int32_T fifo_empty(const fifo_T *fifo);
static int32_T fifo_full(const fifo_T *fifo);
static int32_T fifo_index_add(const fifo_T *fifo, int32_T index);

/*********************************** 全局变量 **********************************/
/*********************************** 接口函数 **********************************/
/*******************************************************************************
*
* 函数名  : fifo_init
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: fifo初始化
*
* 输入参数: fifo fifo变量
*           size fifo大小
*
* 输出参数: 无
* 返回值  : 无
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void fifo_init(fifo_T *fifo, int32_T size)
{
    /* 尾部加哨兵 '\0' */
    uint8_T *buf = (uint8_T *)malloc(size + 1);
    if(NULL == buf)
    {
        while(1);
    } 
    
    fifo->buf = buf;
    fifo->size = size;
    fifo_clear(fifo);
}

/*******************************************************************************
*
* 函数名  : fifo_clear
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: fifo清空
*
* 输入参数: fifo fifo变量
*
* 输出参数: 无
* 返回值  : 无
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void fifo_clear(fifo_T *fifo)
{
    fifo->head = 0;
    fifo->tail = 0;

    /* 尾部加哨兵 '\0' */
    for(int i=0; i < fifo->size + 1; i++)
    {
        fifo->buf[i] = 0;
    }
}


/*******************************************************************************
*
* 函数名  : fifo_write
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: fifo写
*
* 输入参数: buf  写入缓存首地址
*           size 写入数据大小
*
* 输出参数: 无
* 返回值  : 无
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
void fifo_write(fifo_T *fifo, const uint8_T *buf, int32_T size)
{
    int32_T i = 0;

    if(NULL == buf)
    {
        return;
    }

    for(i = 0 ; i < size; i++)
    {
        fifo->buf[fifo->tail] = buf[i];
        fifo->tail = fifo_index_add(fifo, fifo->tail);
        if(fifo_full(fifo))
        {
            break;
        }
    }

}

/*******************************************************************************
*
* 函数名  : fifo_read
* 负责人  : 彭鹏
* 创建日期: 20160624
* 函数功能: fifo读
*
* 输入参数: buf  读取缓存首地址
*           size 读取数据大小
*
* 输出参数: 无
* 返回值  : 实际读取个数
*           小于size表示已经全部读取
* 调用关系: 无
* 其 它   : 无
*
******************************************************************************/
int32_T fifo_read(fifo_T *fifo, uint8_T *buf, int32_T size)
{
    int32_T i = 0;

    if(fifo_empty(fifo))
    {
        return 0;
    }

    while(size-- > 0)
    {
        buf[i++] = fifo->buf[fifo->head];
        fifo->head = fifo_index_add(fifo, fifo->head);
        if(fifo_empty(fifo))
        {
            break;
        }
    }

    return i;
}

/* fifo 满? */
static int32_T fifo_full(const fifo_T *fifo)
{

    if(0 == fifo_space(fifo))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* fifo 空? */
static int32_T fifo_empty(const fifo_T *fifo)
{

    if(fifo->size == fifo_space(fifo))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* fifo获取内部缓存大小 */
static int32_T fifo_size(const fifo_T *fifo)
{
    return fifo->size;
}

/* fifo获取剩余缓存大小 */
static int32_T fifo_space(const fifo_T *fifo)
{
    /* head从0 到 fifo->size head指向当前头 */
    /* tail从0 到 fifo->size tail指向下一个write byte的地址 */

    /*
     * 状态         特征                        剩余大小
     * 空           head == tail                size
     * 满           head == (1 + tail) % size   0
     * 正常(未回滚) head < tail                 size - tail + head
     * 正常(回滚)   head > tail                 size - head + tail
     *
     * */


    /* tail尚未回滚 */
    if( fifo->head == fifo->tail )
    {
        return fifo->size;
    }
    else if( fifo->head == fifo_index_add(fifo, fifo->tail))
    {
        return 0;
    }
    else if( fifo->head < fifo->tail)
    {
        return (fifo->size - fifo->tail + fifo->head);
    }
    else if( fifo->head > fifo->tail)
    {
        return (fifo->size - fifo->head + fifo->tail);
    }

    /* 不应达 */
    while(1);
}

/* 模加 */
static int32_T fifo_index_add(const fifo_T *fifo, int32_T index)
{
    index = ( index + 1 ) % fifo_size(fifo);
    return index;
}

