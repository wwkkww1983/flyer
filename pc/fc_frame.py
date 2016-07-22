#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct
import binascii

#self.mLen = struct.pack('>I', fLen) 
#self.mData = struct.pack('>I', fData)

class FCFrame():
    def __init__(self, fType, fLen, fData):
        super(FCFrame, self).__init__() 
        self.mType = fType
        self.mLen = struct.pack('>I', fLen)
        self.mData = struct.pack('>I', fData)
        self.mCrc32 = None

    def ComputeCrc32(self):
        buf = self.mType + self.mLen + self.mData
        #print(buf)
        #print(len(buf) % 4)
        if 0 != (len(buf) % 4):
            print("不是4字节整数倍,出错.")
            exit()
        crc32 = (binascii.crc32(buf) & 0xffffffff)
        self.mCrc32 = struct.pack('>i', crc32)
        #print("0x%x" % crc32)
        #print(self.mCrc32)

    def GetBytes(self):
        self.ComputeCrc32()
        buf = self.mType + self.mLen + self.mData + self.mCrc32
        return buf

    def Print(self):
        print("type: ", end = '')
        print(self.mType)
        print("len:  ", end = '')
        print(self.mLen)
        print("data: ", end = '')
        print(self.mData)
        print("crc32:", end = '')
        print(self.mCrc32)

if __name__ == '__main__': 
    frame = FCFrame()

    frame.Print()

