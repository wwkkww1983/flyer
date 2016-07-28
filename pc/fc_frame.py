#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from enum import Enum

class FCFrameType(Enum):
    # 以下类型基本帧类型(用户不使用)
    _Up             = 0x80000000
    _Down           = 0x00000000
    _Ctrl           = 0x40000000
    _NoCtrl         = 0x00000000
    _Capture        = 0x20000000
    _NoCapture      = 0x00000000
    _Text           = 0x10000000
    _NoText         = 0x00000000

    _DataPress      = 0x00000020
    _NoDataPress    = 0x00000000
    _DataCompass    = 0x00000010
    _NoDataCompass  = 0x00000000
    _DataGyro       = 0x00000008
    _NoDataGyro     = 0x00000000
    _DataAccel      = 0x00000004
    _NoDataAccel    = 0x00000000
    _DataDmpQuat    = 0x00000002
    _NoDataDmpQuat  = 0x00000000
    _DataTime       = 0x00000001
    _NoDataTime     = 0x00000000

    # 以下类型用户使用
    # dmp四元数采集帧
    FrameRequestTimeAndDmpQuat = _Down | _Capture | _DataDmpQuat | _DataTime
    # 文本输出帧
    FramePrintText = _Up | _Text
    # 错误帧
    FrameError = 0xffffffff

class _FCBaseFrame():
    def __init__(self):
        super(_FCBaseFrame, self).__init__()

    def Print(self):
        print("类型:  ", end = '')
        print(self.Type())
        print("type:  ", end = '')
        if None == self.mType:
            print("None")
        else:
            _FCBaseFrame.PrintBytes(self.mType) 

        print("len:   ", end = '')
        if None == self.mLen:
            print("None")
        else:
            _FCBaseFrame.PrintBytes(self.mLen)

        print("data:  ", end = '')
        if None == self.mData:
            print("None")
        else:
            _FCBaseFrame.PrintBytes(self.mData)

        print("crc32: ", end = '')
        if None == self.mCrc32:
            print("None")
        else:
            _FCBaseFrame.PrintBytes(self.mCrc32)


    def isLenValid(self):
        buf = self.mData
        dataLen = len(buf)
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
        dataLen = dataLen - 1 #除去 crc32的长度
        dataUnpackStr = '<' + 'I' * dataLen
        #_FCBaseFrame.PrintBytes(self.mLen)
        #print(dataLen)
        #print(dataUnpackStr)
        #print(len(self.mData))

        unpackTuple = struct.unpack(dataUnpackStr, self.mData)
        for i in range(0, dataLen):
            val = unpackTuple[i]
            data.append(val) 
            #_FCBaseFrame.PrintBytes(struct.pack('>I', val))
            #print()

        return data

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

# 下行帧 上位机构造
class _FCDownFrame(_FCBaseFrame):
    # 抽象类 用户不使用
    def __init__(self, frameType = None , dataLen = 8, frameData = 0):
        super(_FCDownFrame, self).__init__()

        if dataLen < 4: # 小于4 则无数据
            msgBox = QMessageBox();
            msgBox.setText("下行帧构造失败:数据长至少为4,实际为:%d.\n" % dataLen)
            msgBox.exec_();

        self.mType = struct.pack('>I', frameType.value)
        self.mLen = struct.pack('>I', dataLen)
        self.mData = struct.pack('>I', frameData)
        self.mCrc32 = None

    def GetBytes(self):
        wordList = self.GetCrc32WordList()
        crc32 = _FCBaseFrame.CalStm32Crc32(wordList)
        self.mCrc32 = struct.pack('>I', crc32)
        buf = self.mType + self.mLen + self.mData + self.mCrc32

        return buf

class FCRequestTimeAndDmpQuatFrame(_FCDownFrame):
    def __init__(self, time = 100):
        super(FCRequestTimeAndDmpQuatFrame, self).__init__(frameType = FCFrameType.FrameRequestTimeAndDmpQuat, 
                frameData =time)
    def Type(self):
        return FCFrameType.FrameRequestTimeAndDmpQuat

# 上行帧 上位机解析
class FCUpFrame(_FCBaseFrame):
    def __init__(self, frameBuf = None):
        super(FCUpFrame, self).__init__()
        frameLen = len(frameBuf) 
        #print(frameLen)
        #_FCBaseFrame.PrintBytes(frameBuf)
        if frameLen < 16: # type + len + data + crc
            msgBox = QMessageBox();
            msgBox.setText("上行帧解析失败:帧长至少为16,实际为:%d.\n" % frameLen)
            msgBox.exec_();

        self.mType = frameBuf[0:4]
        self.mLen = frameBuf[4:8]
        self.mData = frameBuf[8:-4]
        self.mCrc32 = frameBuf[-4:]

    def Type(self): 
        # 错误帧
        if not frame.isValid():
            return FCFrameType.FrameError

        frameTypeValue = struct.unpack('>I', self.mType)[0]
        typeEnum = FCFrameType(frameTypeValue)
        #print(frameTypeValue)
        #print(FCFrameType.FrameRequestTimeAndDmpQuat.value)
        #print(typeEnum)
        return typeEnum

    def isValid(self):
        # 校验
        wordList = self.GetCrc32WordList()
        calCrc32 = _FCBaseFrame.CalStm32Crc32(wordList)
        recvCrc32 = struct.unpack('>I', self.mCrc32)[0]

        #print("计算crc", end = ':')
        #_FCBaseFrame.PrintBytes(struct.pack('>I', calCrc32))
        #print("接收crc", end = ':')
        #_FCBaseFrame.PrintBytes(self.mCrc32)
        if recvCrc32 == calCrc32:
            return True
        else:
            return False

    @staticmethod
    def ParseLen(buf):
        bufLen = len(buf)
        if 8 < bufLen:
            msgBox = QMessageBox();
            msgBox.setText("上行帧的帧长解析失败:%d.\n" % bufLen)
            msgBox.exec_();
        dataLen = buf[4:8]
        length = struct.unpack('>I', dataLen)

        return length[0]

if __name__ == '__main__':
    frame = _FCBaseFrame()

    frame.Print()

