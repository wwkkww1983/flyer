#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct
import ctypes

class FCFrame():
    def __init__(self, fType, fLen, fData):
        super(FCFrame, self).__init__() 
        self.mType = fType
        self.mLen = struct.pack('>I', fLen)
        self.mData = struct.pack('>I', fData)
        self.mCrc32 = None
        #self.Print()

    def ComputeCrc32(self):
        buf = self.mType + self.mLen + self.mData
        #self.Print()
        #print(buf)
        #print(len(buf) % 4)
        if 0 != (len(buf) % 4):
            print("不是4字节整数倍,出错.")
            exit()

        # 调用C函数 
        #lib_handle = ctypes.CDLL('pp.dll')
        #lib_handle = ctypes.WinDLL('pp.dll')
        ctypes.cdll.LoadLibrary("crc32.dll")
        cal_crc32 = lib_handle.cal_crc32 
        cal_crc32.argtypes = [ctypes.c_void_p, ctypes.c_uint]
        cal_crc32.restypes = ctypes.c_uint
        crc32 = cal_crc32(buf, int(len(buf) / len(int)) )
        #print("0x%x" % crc32)

        self.mCrc32 = struct.pack('>I', crc32)
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

