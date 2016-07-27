#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from PyQt5.QtWidgets import *

class FCBaseFrame():
    def __init__(self):
        super(FCBaseFrame, self).__init__()

    def Print(self):
        print("type:  ", end = '')
        FCBaseFrame.PrintBytes(self.mType)
        print("len:   ", end = '')
        FCBaseFrame.PrintBytes(self.mLen)
        print("data:  ", end = '')
        FCBaseFrame.PrintBytes(self.mData)
        print("crc32: ", end = '')
        if None == self.mCrc32:
          print("None")
        else:
          FCBaseFrame.PrintBytes(self.mCrc32) 

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

# 下行帧 上位机构造
class FCDownFrame(FCBaseFrame):
    def __init__(self, frame_type = None , frame_len = 0, frame_data = 0, frame_crc32 = None):
        super(FCDownFrame, self).__init__()

        # 构造下行帧
        self.mType = frame_type
        self.mLen = struct.pack('>I', frame_len)
        self.mData = struct.pack('>I', frame_data)
        self.mCrc32 = frame_crc32

        self.mBuf = None

    def _Pack(self):
        #计算校验值
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

        crc32 = FCBaseFrame.CalStm32Crc32(data)
        self.mCrc32 = struct.pack('>I', crc32)
        buf = self.mType + self.mLen + self.mData + self.mCrc32

        self.mBuf = buf

    def GetBytes(self): 
        # 解析
        self._Pack()
        return self.mBuf

# 上行帧 上位机解析
class FCUpFrame(FCBaseFrame):
    def __init__(self, frame_buf = None):
        super(FCDownFrame, self).__init__()
        self.mBuf = frame_buf

        buf_len = len(frame_buf)
        if buf_len < 12: # type + len + crc
            msgBox = QMessageBox();
            msgBox.setText("上行帧解析失败:帧长至少为12,实际为:%d.\n" % buf_len)
            msgBox.exec_();

        self.mType = self.mBuf[0:4]
        self.mLen = struct.unpack('>I', self.mBuf[4:8])
        self.mData = struct.pack('>I', self.mBuf)
        self.mCrc32 = frame_crc32

    @staticmethod
    def ParseLen(buf):
        buf_len = len(buf)
        if 8 < buf_len:
            msgBox = QMessageBox();
            msgBox.setText("上行帧长解析失败:%d.\n" % buf_len)
            msgBox.exec_();
        len_buf = buf[4:8]
        length = struct.unpack('>I', len_buf)

        return length

if __name__ == '__main__':
    frame = FCBaseFrame()

    frame.Print()


