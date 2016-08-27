#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from algo.quat import FCQuat

from frame.frame_type import *
from frame.frame_base import * 

##################################################################################################
# 抽象类
# 上行帧 上位机解析
class FCUpFrame(FCBaseFrame): 
    def __init__(self, frameBuf):
        super(FCUpFrame, self).__init__()

        frameLen = len(frameBuf) 
        #print(frameLen)
        #FCBaseFrame.PrintBytes(frameBuf)
        if frameLen < 16: # type + len + data + crc
            print("上行帧解析失败:帧长至少为16,实际为:%d.\n" % frameLen)

        self.mType = frameBuf[0:4]
        self.mLen = frameBuf[4:8]
        self.mData = frameBuf[8:-4]
        self.mCrc32 = frameBuf[-4:]

    @staticmethod
    def ParseLen(buf):
        bufLen = len(buf)
        if 8 < bufLen:
            print("上行帧的帧长解析失败:%d.\n" % bufLen)
        dataLen = buf[4:8]
        length = struct.unpack('>I', dataLen)

        return length[0]

    @staticmethod
    def Parse(buf):
        # 表驱动 
        upFrameClassDict = {FCFrameType.FramePrintText:                    FCPrintTextFrame,
                            FCFrameType.FrameDataTimeAcceleratorDmpQuat:   FCDataTimeAcceleratorDmpQuat,
                            FCFrameType.FrameDataTimeAcceleratorEulerPid:  FCDataTimeAcceleratorEulerPid}
        # 解析上行帧
        frameTypeValue = struct.unpack('>I', buf[0:4])[0]
        try:
            frameType = FCFrameType(frameTypeValue)
        except Exception as e: # 处理无效类型
            return FCErrorFrame(buf)
        else: # 有效类型 
            # TODO:使用表驱动方案
            if frameType in upFrameClassDict:
                frameClass = upFrameClassDict[frameType]
                #print(frameClass)
                frame = frameClass(buf) 
                if frame.isValid(): 
                    return frame
                else:
                    return FCErrorFrame(buf)
            else:
                return FCErrorFrame(buf)

    def isValid(self):
        # 校验
        wordList = self.GetCrc32WordList()
        calCrc32 = FCBaseFrame.CalStm32Crc32(wordList)
        recvCrc32 = struct.unpack('>I', self.mCrc32)[0]

        #print("计算crc", end = ':')
        #FCBaseFrame.PrintBytes(struct.pack('>I', calCrc32))
        #print("接收crc", end = ':')
        #FCBaseFrame.PrintBytes(self.mCrc32)
        if recvCrc32 == calCrc32:
            return True
        else:
            return False

##################################################################################################
# 具体类
class FCPrintTextFrame(FCUpFrame):
    def __init__(self, frameBuf): 
        super(FCPrintTextFrame, self).__init__(frameBuf)

    def Type(self): 
        return FCFrameType.FramePrintText

    def GetText(self):
        # 清理末尾的填充
        textBuf = self.mData
        i = -1

        while gFillChar == textBuf[i]:
            i = i - 1
        # 仅处理有尾部填充的情况
        if i < -1:
            textBuf = textBuf[:i+1]

        # 生成字符串 
        text = textBuf.decode('utf8')
        return text

class FCDataTimeAcceleratorDmpQuat(FCUpFrame):
    def __init__(self, frameBuf): 
        super(FCDataTimeAcceleratorDmpQuat, self).__init__(frameBuf)

    def Type(self): 
        return FCFrameType.FrameDataTimeAcceleratorDmpQuat

    def GetTime(self):
        timeBuf = self.mData[0:4]
        interval = struct.unpack('>I', timeBuf)[0]
        return interval

    def GetGmpQuat(self):
        dmpQuatBuf = self.mData[4:20]
        dmpQuatTuplle = struct.unpack('>ffff', dmpQuatBuf)

        dmpQuat = FCQuat(*dmpQuatTuplle)

        return dmpQuat

    def GetAccelrator(self):
        accelerator = self.mData[20:]
        acceleratorTuplle = struct.unpack('>IIIII', accelerator)
        return acceleratorTuplle

class FCDataTimeAcceleratorEulerPid(FCUpFrame):
    def __init__(self, frameBuf): 
        super(FCDataTimeAcceleratorEulerPid, self).__init__(frameBuf)

    def Type(self): 
        return FCFrameType.FrameDataTimeAcceleratorEulerPid

    def GetTime(self):
        timeBuf = self.mData[0:4]
        interval = struct.unpack('>I', timeBuf)[0]
        return interval

    def GetAccelrator(self):
        accelerator = self.mData[4:24]
        acceleratorTuplle = struct.unpack('>IIIII', accelerator)
        return acceleratorTuplle

    def GetEuler(self):
        euler = self.mData[24:36]
        eulerTuplle = struct.unpack('>fff', euler)
        return eulerTuplle

    def GetPid(self):
        pid = self.mData[36:48]
        pidTuplle = struct.unpack('>fff', pid)
        return pidTuplle

class FCErrorFrame(FCUpFrame):
    def __init__(self, frameBuf): 
        super(FCErrorFrame, self).__init__(frameBuf)

    def Type(self): 
        return FCFrameType.FrameError

if __name__ == '__main__':
    print("偷懒,先不写单元测试")

