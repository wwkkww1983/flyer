#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct
import ctypes

def CalStm32Crc32(data):
    """
    stm32处理器crc32模块采用的算法软件实现
    """
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

def PrintBytes(bs): 
    for b in bs:
        print("\\x%02x" % b, end = "")
    print()

class FCFrame():
    def __init__(self, fType, fLen, fData):
        super(FCFrame, self).__init__() 
        self.mType = fType
        self.mLen = struct.pack('>I', fLen)
        self.mData = struct.pack('>I', fData)
        self.mCrc32 = None
        #self.Print()

    def GetBytes(self):
        """
        计算校验值
        """
        buf = self.mType + self.mLen + self.mData
        if 0 != (len(buf) % 4):
            print("不是4字节整数倍,出错.")
            exit()

        # 构造32bit字列表
        data = []
        unpackTuple = struct.unpack('<I', self.mType)
        idata = unpackTuple[0]
        data.append(idata)
        unpackTuple = struct.unpack('<I', self.mLen)
        idata = unpackTuple[0]
        data.append(idata)
        unpackTuple = struct.unpack('<I', self.mData)
        idata = unpackTuple[0]
        data.append(idata)

        crc32 = CalStm32Crc32(data) 
        self.mCrc32 = struct.pack('>I', crc32)
        buf = self.mType + self.mLen + self.mData + self.mCrc32

        return buf

    def Print(self):
        print("type:  ", end = '')
        PrintBytes(self.mType)
        print("len:   ", end = '')
        PrintBytes(self.mLen)
        print("data:  ", end = '')
        PrintBytes(self.mData)
        print("crc32: ", end = '')
        PrintBytes(self.mCrc32)


if __name__ == '__main__': 
    frame = FCFrame()

    frame.Print()


