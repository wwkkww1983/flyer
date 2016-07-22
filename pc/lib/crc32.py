#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

def cal_stm32_crc32(data):
    xbit = 0
    # 复位值为全1
    crc = 0xFFFFFFFF
    polyNomial = 0x04C11DB7

    for d in data:
        xbit = 1 << 31
        for i in range(0, 32):
            if 0x80000000 & crc:
                crc = crc << 1
                crc = crc ^ polyNomial
            else:
                crc = crc << 1

            if d & xbit:
                crc = crc ^ polyNomial
            
            xbit = xbit >> 1

    # 截断 32bit
    return crc & 0xffffffff

if __name__ == '__main__': 
    data = []
    data.append(0x00001e00)
    data.append(0x00000e00)
    data.append(0x98fff609)
    data.append(0xc83ea800)
    data.append(0x0e00f7ff)
    data.append(0xffa90000)
    data.append(0xff99ffb1)

    crc32 = cal_stm32_crc32(data)
    print("计算crc32:%08x" % crc32);
    print("传输crc32:d5b726ea");

