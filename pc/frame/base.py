#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct

gFillChar = 165
gFillByte = b'\xA5' 

class FCBaseFrame():
    def __init__(self):
        super(FCBaseFrame, self).__init__()

    def Print(self):
        print("类型:  ", end = '')
        if None == self.Type():
            print("None")
        else:
            print(self.Type())

        print("type:  ", end = '')
        if None == self.mType:
            print("None")
        else:
            FCBaseFrame.PrintBytes(self.mType) 

        print("len:   ", end = '')
        if None == self.mLen:
            print("None")
        else:
            FCBaseFrame.PrintBytes(self.mLen)

        print("data:  ", end = '')
        if None == self.mData:
            print("None")
        else:
            FCBaseFrame.PrintBytes(self.mData)

        print("crc32: ", end = '')
        if None == self.mCrc32:
            print("None")
        else:
            FCBaseFrame.PrintBytes(self.mCrc32)


    def isLenValid(self):
        buf = self.mData
        dataLen = len(buf)
        if 0 == (dataLen % 4):
            return True
        else:
            print("错误帧如下:%d." % dataLen)
            self.Print()
            while True:
                pass
            return False

    def GetCrc32WordList(self):
        if not self.isLenValid():
            print("帧长度错误.")
            exit()

        # 构造32bit字列表
        data = []
        unpackTuple = struct.unpack('<I', self.mType)
        iType = unpackTuple[0]
        data.append(iType)

        unpackTuple = struct.unpack('<I', self.mLen)
        iLen = unpackTuple[0]
        data.append(iLen)

        # crc32校验使用的字节序与解析使用相反的大小端
        dataLen = int(struct.unpack('>I', self.mLen)[0] / 4)
        dataLen = dataLen - 1 #除去 crc32的长度
        dataUnpackStr = '<' + 'I' * dataLen
        #FCBaseFrame.PrintBytes(self.mLen)
        #print(dataLen)
        #print(dataUnpackStr)
        #print(len(self.mData))

        unpackTuple = struct.unpack(dataUnpackStr, self.mData)
        for i in range(0, dataLen):
            val = unpackTuple[i]
            data.append(val) 
            #FCBaseFrame.PrintBytes(struct.pack('>I', val))
            #print()

        return data

    def GetBytes(self):
        buf = self.mType + self.mLen + self.mData + self.mCrc32
        return buf

    @staticmethod
    def PackType(frameType):
        return struct.pack('>I', frameType)

    @staticmethod
    def PrintBytes(bs):
        for b in bs:
            print("\\x%02x" % b, end = "")
        print()

    @staticmethod
    def CalStm32Crc32(data):
        #stm32处理器crc32模块采用的算法软件实现
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

