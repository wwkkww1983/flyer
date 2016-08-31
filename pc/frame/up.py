#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from frame.data.quat import FCQuat

from frame.type import *
from frame.base import * 

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
                frame = frameClass(buf) 
                if frame.isValid(): 
                    return frame
                else:
                    return FCErrorFrame(buf)
            else:
                return FCErrorFrame(buf)

    def GetTime(self):
        # 上行帧必有时间
        timeBuf = self.mData[0:4]
        interval = struct.unpack('>I', timeBuf)[0]
        return interval

    def GetText(self):
        # print(FCFrameType.FramePrint)
        # print(self.Type())
        # 该帧有字符串则解析
        if not FCFrameType.FramePrint.value & self.Type().value:
            return None

        # 清理末尾的填充
        textBuf = self.mData
        i = -1

        while gFillChar == textBuf[i]:
            i = i - 1
        # 仅处理有尾部填充的情况(去掉4bytes时间头和填充尾部)
        if i < -1:
            textBuf = textBuf[4:i+1]

        # 生成字符串 
        text = textBuf.decode('utf8')
        return text

    def GetDmpQuat(self):
        print('GetGmpQuat 未实现')
        return None

    def GetAccel(self):
        print('GetAccel 未实现')
        return None

    def GetGyro(self):
        print('GetGyro 未实现')
        return None

    def GetCompass(self):
        print('GetCompass 未实现')
        return None

    def GetPress(self):
        print('GetPress 未实现')
        return None

    def GetAccelerator(self):
        print('GetAccelerator 未实现')
        return None

    def GetEuler(self):
        print('GetEuler 未实现')
        return None

    def GetPid(self):
        print('GetPid 未实现')
        return None

    def Dict(self):
        # TODO:继续实现
        allDict = {}
        frameDict = {}

        frameDict['text'] = self.GetText()
        frameDict['dmpQuat'] = self.GetDmpQuat()
        frameDict['accel'] = self.GetAccel()
        frameDict['gyro'] = self.GetGyro()
        frameDict['compass'] = self.GetCompass()
        frameDict['press'] = self.GetPress()
        frameDict['accelerator'] = self.GetAccelerator()
        frameDict['euler'] = self.GetEuler()
        frameDict['pid'] = self.GetPid()

        time = self.GetTime()
        allDict[time] = frameDict

        return allDict

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

    def Type(self):
        frameTypeValue = struct.unpack('>I', self.mType)[0]
        try: 
            frameType = FCFrameType(frameTypeValue)
        except Exception as e: # 无效类型
            return FCFrameType.FrameError
        else:                  # 有效类型 
            return frameType

##################################################################################################
# 具体类 不使用
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

