/******************************************************************************
 *
 * 文件名  ： test.c
 * 负责人  ： 彭鹏(pengpeng@fiberhome.com)
 * 创建日期： 20150116
 * 版本号  ： v1.0
 * 文件描述： 测试lib中的库
 * 版权说明： Copyright (c) 2000-2020   烽火通信科技股份有限公司
 * 其    他： 无
 * 修改日志： 无
 *
 *******************************************************************************/


/*---------------------------------- 预处理区 ---------------------------------*/

/************************************ 头文件 ***********************************/
#include <stdio.h>
#include <string.h>

#include "fifo.h"



/*----------------------------------- 声明区 ----------------------------------*/

/********************************** 变量声明区 *********************************/


/********************************** 函数声明区 *********************************/


/********************************** 变量实现区 *********************************/


/********************************** 函数实现区 *********************************/
/*******************************************************************************
 *
 * 函数名  : main
 * 负责人  : 彭鹏
 * 创建日期: 无
 * 函数功能: 主函数
 *
 * 输入参数: argc - 参数个数
 *           argv - 命令行参数数组
 *
 * 输出参数: 无
 *
 * 返回值:   0   : 正常退出
 *           其它: 异常退出
 * 调用关系: 无
 * 其 它:    无
 *
 ******************************************************************************/
static fifo_T s_fifo;
int main(int argc, char *argv[])
{
    uint8_T buf_w[] = "12";
    uint8_T buf_r[1024] = {0};
    int32_T read_bytes = 0;
    int32_T i = 0;
    uint8_T *ptr_buf = NULL;
    memset(buf_r, 0, 1024);

    fifo_init(&s_fifo, 4);

    /* 空读 */
    fifo_write(&s_fifo, buf_w, 2);
    read_bytes = fifo_read(&s_fifo, buf_r, 1);
    if( (1 != read_bytes)
     || ('1' != buf_r[0]))
    {
        printf("错开读取失败!\r\n");
        return 0;
    }
    read_bytes = fifo_read(&s_fifo, buf_r, 1);
    if( (1 != read_bytes)
     || ('2' != buf_r[0]))
    {
        printf("读最后1Bytes出错!\r\n");
        return 0;
    }
    read_bytes = fifo_read(&s_fifo, buf_r, 1);
    if( (0 != read_bytes)
     || ('2' != buf_r[0])) /* 保持上次的值 */
    {
        printf("读空fifo出错!\r\n");
        return 0;
    }

    fifo_clear(&s_fifo);
    /* 读取速度比写入快 */
    memset(buf_r, 0, 1024);
    ptr_buf = buf_r;
    i = 0;
    while( i++ < 100 )
    {
        fifo_write(&s_fifo, buf_w, 2);

        read_bytes = fifo_read(&s_fifo, ptr_buf, 3);
        if( (2 != read_bytes))
        {
            printf("%s,%d:读取数量不对!\r\n", __FILE__, __LINE__);
            return 0;
        }

        ptr_buf += 2;
    }

    fifo_clear(&s_fifo);
    /* FIXME: 会丢包 */
    /* 写入速度比读取快 */
    memset(buf_r, 0, 1024);
    ptr_buf = buf_r;
    while( i++ < 10 )
    {
        fifo_write(&s_fifo, buf_w, 2);
        read_bytes = fifo_read(&s_fifo, ptr_buf, 1);
        if( (1 != read_bytes))
        {
            printf("%s,%d:读取数量不对!\r\n", __FILE__, __LINE__);
            return 0;
        }

        ptr_buf += 2;
    }

    return 0;

}

void fifo_init(fifo_T *fifo, int32_t size);
void fifo_write(fifo_T *fifo, const uint8_T *buf, int32_t size);
int32_T fifo_read(fifo_T *fifo, uint8_T *buf, int32_t size);


