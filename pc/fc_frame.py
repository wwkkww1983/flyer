#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from PyQt5.QtWidgets import *

class FCFrameType():
    # 以下类型用户不使用
    FrameUP             = 0x80000000
    FrameDown           = 0x00000000
    FrameCtrl           = 0x40000000
    FrameNoCtrl         = 0x00000000
    FrameCapture        = 0x20000000
    FrameNoCapture      = 0x00000000
    FrameText           = 0x10000000
    FrameNoText         = 0x00000000

    FrameDataPress      = 0x00000020
    FrameNoDataPress    = 0x00000000
    FrameDataCompass    = 0x00000010
    FrameNoDataCompass  = 0x00000000
    FrameDataGyro       = 0x00000008
    FrameNoDataGyro     = 0x00000000
    FrameDataAccel      = 0x00000004
    FrameNoDataAccel    = 0x00000000
    FrameDataDmpQuat    = 0x00000002
    FrameNoDataDmpQuat  = 0x00000000
    FrameDataTime       = 0x00000001
    FrameNoDataTime     = 0x00000000

    # 以下类型用户使用
    # dmp四元数采集帧
    FrameRequestTimeAndDmpQuat = struct.pack('>I', FrameDown | FrameCapture | FrameDataDmpQuat | FrameDataTime)

class FCBaseFrame():
    # 抽象类用户不使用
    def __init__(self):
        super(FCBaseFrame, self).__init__()

    def Print(self):
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
        #print(len(buf))
        if 0 == (dataLen % 4):
            return True
        else:
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
        dataUnpackStr = '<' + 'I' * dataLen
        #FCBaseFrame.PrintBytes(self.mLen)
        #print(dataLen)
        #print(dataUnpackStr)

        unpackTuple = struct.unpack(dataUnpackStr, self.mData)
        idata = unpackTuple[0]
        data.append(idata)

        return data

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
    def __init__(self, frame_type = None , dataLen = 4, frameData = 0):
        super(FCDownFrame, self).__init__()

        if dataLen < 4: # 小于4 则无数据
            msgBox = QMessageBox();
            msgBox.setText("下行帧构造失败:数据长至少为4,实际为:%d.\n" % dataLen)
            msgBox.exec_();

        # 构造下行帧
        self.mType = frame_type
        self.mLen = struct.pack('>I', dataLen)
        self.mData = struct.pack('>I', frameData)
        self.mCrc32 = None

    def GetBytes(self):
        wordList = self.GetCrc32WordList()
        crc32 = FCBaseFrame.CalStm32Crc32(wordList)
        self.mCrc32 = struct.pack('>I', crc32)
        buf = self.mType + self.mLen + self.mData + self.mCrc32

        return buf

# 上行帧 上位机解析
class FCUpFrame(FCBaseFrame):
    def __init__(self, frameBuf = None):
        super(FCDownFrame, self).__init__()
        frameLen = len(frameBuf)
        if frameLen < 16: # type + len + data + crc
            msgBox = QMessageBox();
            msgBox.setText("上行帧解析失败:帧长至少为16,实际为:%d.\n" % frameLen)
            msgBox.exec_();

        self.mType = frameBuf[0:4]
        self.mLen = frameBuf[4:8]
        self.mData = frameBuf[8:-4]
        self.mCrc32 = frameBuf[-4:-1]

    def isValid(self):
        #计算校验值
        wordList = self.GetCrc32WordList()
        calCrc32 = FCBaseFrame.CalStm32Crc32(wordList)
        recvCrc32 = struct.pack('>I', self.mCrc32)

        # TODO:比较是否一样
        print(calCrc32)
        print(recvCrc32)

    @staticmethod
    def ParseLen(buf):
        bufLen = len(buf)
        if 8 < bufLen:
            msgBox = QMessageBox();
            msgBox.setText("上行帧的帧长解析失败:%d.\n" % bufLen)
            msgBox.exec_();
        dataLen = buf[4:8]
        length = struct.unpack('>I', dataLen)

        return length

if __name__ == '__main__':
    frame = FCBaseFrame()

    frame.Print()


