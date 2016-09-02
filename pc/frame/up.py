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

# 尚未实现
gAccelLen = 12
gGyroLen = 12
gCompassLen = 12
gPressLen = 4 


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

        self.mDataDict = { FCFrameType.FrameDmpQuat: (FCQuat, '>ffff', 20),
                }

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
        dmpQuat = self.MakeDataObj(FCFrameType.FrameDmpQuat)
        return dmpQuat

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

    def MakeDataObj(self, dataEnum):
        if not self.HasField(dataEnum):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print(dataEnum, end = '')
            print("数据和文本不可共存.")
            return None

        dataClass = self.mDataDict[dataEnum][0]
        unpackStr = self.mDataDict[dataEnum][1]
        dataLen = self.mDataDict[dataEnum][2]

        offset = self.GetDataOffset(dataEnum)
        byteBuf = self.mData[offset : offset + dataLen]
        dataTuplle = struct.unpack(unpackStr, byteBuf)

        data = dataClass(*dataTuplle)
        return data

    def GetDataOffset(self, field):
        if FCFrameType.FramePrint == field:
            print('文本帧不可调用GetDataOffset.')
            return None

        # 以下是数据帧
        offset = gTimeLen # 时间数据必有

        if FCFrameType.FrameDmpQuat == field:
            return offset

        # 按照条件处理dmpQuat偏移
        if self.HasField(FCFrameType.FrameDmpQuat):
            offset += gDmpQuatLen

        if FCFrameType.FrameAccel == field:
            return offset

        # 按照条件处理accel偏移
        if self.HasField(FCFrameType.FrameAccel):
            offset += gAccelLen

        if FCFrameType.FrameGyro == field:
            return offset

        # 按照条件处理gyro偏移
        if self.HasField(FCFrameType.FrameGyro):
            offset += gGyroLen

        if FCFrameType.FrameCompass == field:
            return offset

        # 按照条件处理compass偏移
        if self.HasField(FCFrameType.FrameCompass):
            offset += gCompassLen

        if FCFrameType.FramePress == field:
            return offset

        # 按照条件处理Press偏移
        if self.HasField(FCFrameType.FramePress):
            offset += gPressLen

        if FCFrameType.FrameAccelerator == field:
            return offset

        # 按照条件处理Accelerator偏移
        if self.HasField(FCFrameType.FrameAccelerator):
            offset += gAcceleratorLen

        if FCFrameType.FrameEuler == field:
            return offset

        # 按照条件处理Euler偏移
        if self.HasField(FCFrameType.FrameEuler):
            offset += gEulerLen

        if FCFrameType.FramePid == field:
            return offset 
        
        print('GetDataOffset参数field错误,field为:', end = '')
        print(field)
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

    def GetCalCrc32(self):
        wordList = self.GetCrc32WordList()
        calCrc32 = FCBaseFrame.CalStm32Crc32(wordList)

        return calCrc32

    def isValid(self):
        frameType = self.Type()
        if frameType.FrameError:
            return False

        # 校验
        calCrc32 = self.GetCalCrc32()
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

