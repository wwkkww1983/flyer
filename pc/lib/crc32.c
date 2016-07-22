#include <stdio.h>

unsigned int cal_crc32(unsigned int *ptr, unsigned int len)
{
    unsigned int xbit;
    unsigned int data;
    unsigned int crc = 0xFFFFFFFF;    /* 复位值全1 */
    unsigned int dwPolynomial = 0x04c11db7;
    while (len--)
    {
        xbit = 1 << 31;

        data = *ptr++;
        for (int bits = 0; bits < 32; bits++) 
        {
            if (crc & 0x80000000) 
            {
                crc <<= 1;
                crc ^= dwPolynomial;
            }
            else
            {
                crc <<= 1;
            }

            if (data & xbit)
            {
                crc ^= dwPolynomial;
            }

            xbit >>= 1;
        }
    }

    return crc;
}

int main(void)
{
    /* 小端(逆序) */
    unsigned int data[] = 
    {
        0x00001e00,
        0x00000e00,
        0x98fff609,
        0xc83ea800,
        0x0e00f7ff,
        0xffa90000,
        0xff99ffb1
    };

    unsigned int crc32 = 0;


    crc32 = cal_crc32(data, sizeof(data) / sizeof(unsigned int));

    printf("compute crc32:%08x\r\n", crc32);
    printf("right crc32:  d5b726ea\r\n");

    return 0;
}

