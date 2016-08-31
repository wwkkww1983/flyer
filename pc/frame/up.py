#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from frame.data.quat import FCQuat

from frame.type import *
from frame.base import * 

# 上行帧中各字段的字节数
gTimeLen = 4
gDmpQuatLen = 16
gAcceleratorLen = 20
gEulerLen = 12
gPidLen = 12

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

    def GetTime(self):
        # 上行帧必有时间
        timeBuf = self.mData[0 : gTimeLen]
        interval = struct.unpack('>I', timeBuf)[0]

        return interval

    def HasField(self, field):
        if self.Type() & field.value:
            return True

    def GetText(self):
        if not self.HasField(FCFrameType.FramePrint):
            return None

        if self.HasField(FCFrameType.FrameDataAll):
            print("文本帧中包含采样数据.")
            return None

        textBuf = self.mData[gTimeLen:]
        # 清理末尾的填充
        i = -1
        while gFillChar == textBuf[i]:
            i = i - 1
        # 仅处理有尾部填充的情况(去掉4bytes时间头和填充尾部)
        if i < -1:
            textBuf = textBuf[:i+1]

        # 生成字符串 
        text = textBuf.decode('utf8')
        return text

    def GetDmpQuat(self):
        if not self.HasField(FCFrameType.FrameDmpQuat):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print("采样数据帧中包含文本.")
            return None

        offset = 0
        dmpQuat = self.mData[offset : offset + gDmpQuatLen]

        print('GetGmpQuat 未实现')
        return None

    def GetAccel(self):
        if not self.HasField(FCFrameType.FrameAccel):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print("采样数据帧中包含文本.")
            return None

        print('GetAccel 未实现')
        return None

    def GetGyro(self):
        if not self.HasField(FCFrameType.FrameGyro):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print("采样数据帧中包含文本.")
            return None

        print('GetGyro 未实现')
        return None

    def GetCompass(self):
        if not self.HasField(FCFrameType.FrameCompass):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print("采样数据帧中包含文本.")
            return None

        print('GetCompass 未实现')
        return None

    def GetPress(self):
        if not self.HasField(FCFrameType.FramePress):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print("采样数据帧中包含文本.")
            return None

        print('GetPress 未实现')
        return None

    def GetAccelerator(self):
        # 打印帧不可和采样帧共存
        if not self.HasField(FCFrameType.FrameAccelerator):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print("采样数据帧中包含文本.")
            return None

        print('GetAccelerator 未实现')
        return None

    def GetEuler(self):
        if not self.HasField(FCFrameType.FrameEuler):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print("采样数据帧中包含文本.")
            return None

        print('GetEuler 未实现')
        return None

    def GetPid(self):
        if not self.HasField(FCFrameType.FramePid):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print("采样数据帧中包含文本.")
            return None

        print('GetPid 未实现')
        return None

    def Dict(self):
        # TODO:继续实现
        allDict = {}
        frameDict = {}

        time = self.GetTime()
        #print(time)

        frameDict['text'] = self.GetText()
        frameDict['dmpQuat'] = self.GetDmpQuat()
        frameDict['accel'] = self.GetAccel()
        frameDict['gyro'] = self.GetGyro()
        frameDict['compass'] = self.GetCompass()
        frameDict['press'] = self.GetPress()
        frameDict['accelerator'] = self.GetAccelerator()
        frameDict['euler'] = self.GetEuler()
        frameDict['pid'] = self.GetPid()
        allDict[time] = frameDict

        return allDict

    def isValid(self):
        frameType = self.Type()
        if frameType.FrameError:
            return False

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
        return frameTypeValue
        """
        try: 
            frameType = FCFrameType(frameTypeValue)
        except Exception as e: # 无效类型
            return FCFrameType.FrameError
        else:                  # 有效类型 
            return frameType
        """

if __name__ == '__main__':
    print("偷懒,先不写单元测试")

