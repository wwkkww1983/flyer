#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct
import json

from frame.data.quat import FCQuat
from frame.data.euler import FCEuler
from frame.data.accelerator import FCAccelerator
from frame.data.pid import FCPid
from frame.data.accel import FCAccel

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

        self.mDataDict = { 
                FCFrameType.FrameDmpQuat: (FCQuat, '>ffff', 16),
                FCFrameType.FrameAccel: (FCAccel, '>fff', 12),
                FCFrameType.FrameGyro: None,
                FCFrameType.FrameCompass: None,
                FCFrameType.FramePress: None,
                FCFrameType.FrameAccelerator: (FCAccelerator, '>IIIII', 20),
                FCFrameType.FrameEuler: (FCEuler, '>fff', 12),
                FCFrameType.FramePid: (FCPid, '>fff', 12)}

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
        accel = self.MakeDataObj(FCFrameType.FrameAccel)
        return accel

    def GetGyro(self):
        gyro = self.MakeDataObj(FCFrameType.FrameGyro)
        return gyro

    def GetCompass(self):
        compass = self.MakeDataObj(FCFrameType.FrameCompass)
        return compass

    def GetPress(self):
        press = self.MakeDataObj(FCFrameType.FramePress)
        return press

    def GetAccelerator(self): 
        accelerator = self.MakeDataObj(FCFrameType.FrameAccelerator)
        return accelerator

    def GetEuler(self):
        euler = self.MakeDataObj(FCFrameType.FrameEuler)
        return euler

    def GetPid(self):
        pid = self.MakeDataObj(FCFrameType.FramePid)
        return pid

    def MakeDataObj(self, dataEnum):
        if not self.HasField(dataEnum):
            return None

        if self.HasField(FCFrameType.FramePrint):
            print(dataEnum, end = '')
            print("数据和文本不可共存.")
            return None

        # 字典中尚未实现该数据
        if not (dataEnum in self.mDataDict):
            return None

        dataClass = self.mDataDict[dataEnum][0]
        unpackStr = self.mDataDict[dataEnum][1]
        dataLen = self.mDataDict[dataEnum][2]
        #print(dataClass)
        #print(unpackStr)
        #print(dataLen)

        offset = self.GetDataOffset(dataEnum)
        byteBuf = self.mData[offset : offset + dataLen]
        #print(offset)
        #FCBaseFrame.PrintBytes(byteBuf)
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

    def ToFrameDict(self):
        # TODO:继续实现
        frameDict = {}

        time = self.GetTime()
        #print(time)

        frameDict['文本'] = self.GetText()
        frameDict['DMP四元数'] = self.GetDmpQuat()
        frameDict['加计'] = self.GetAccel()
        frameDict['陀螺仪'] = self.GetGyro()
        frameDict['磁计'] = self.GetCompass()
        frameDict['压力'] = self.GetPress()
        frameDict['油门'] = self.GetAccelerator()
        frameDict['欧拉角'] = self.GetEuler()
        frameDict['PID'] = self.GetPid()

        return (time, frameDict)

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

    def PrintDict(self):
        (time, frameDict) = self.ToFrameDict() 
        if frameDict['text']: 
            print('%05d:%s' % (time, frameDict['text']), end = '')
        else:
            print('%05d:{dmpQuat:%s,accel:%s,gyro:%s,compass:%s,press:%s,accelerator:%s,euler:%s,pid:%s}' % (time, 
                frameDict['dmpQuat'], frameDict['accel'], frameDict['gyro'], frameDict['compass'],
                frameDict['press'], frameDict['accelerator'], frameDict['euler'], frameDict['pid']))

if __name__ == '__main__':
    print("偷懒,先不写单元测试")

