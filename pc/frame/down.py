#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from frame.type import FCFrameType
from frame.base import FCBaseFrame

# 下行帧len字段长度固定
gDownFrameLenVal = 24
# 数据字段打包格式
gDataUnpackStr = '>' + 'I' * 5

#####################################################################################################################
# 抽象类
# 下行帧 上位机构造
class FCDownFrame(FCBaseFrame):
    # 抽象类 用户不使用
    def __init__(self, frameType, frameData):
        super(FCDownFrame, self).__init__()

        self.mType = struct.pack('>I', frameType.value)
        self.mLen = struct.pack('>I', gDownFrameLenVal)
        self.mData = struct.pack(gDataUnpackStr, *frameData)

        # 生产CRC32
        wordList = self.GetCrc32WordList()
        crc32 = FCBaseFrame.CalStm32Crc32(wordList)
        self.mCrc32 = struct.pack('>I', crc32)

# 控制帧
class FCCtrlFrame(FCDownFrame):
    def __init__(self, type_byte = b'\x00', data = b'\xA5\xA5'):
        # byte0       : 0(加速) 1(油门)
        # val         : byte1 << 8 | byte2
        # byte3-byt19 : 0xA5
        byte0 = type_byte
        byte1_byte3 = data
        byte4_byte19 = b'\xA5' * 17
        bytesVal = byte0 + byte1_byte3 + byte4_byte19
        #print(bytesVal)
        #print(gDataUnpackStr)
        data = struct.unpack(gDataUnpackStr, bytesVal)
        #print(data)
        super(FCCtrlFrame, self).__init__(frameType = self.Type(), frameData = data)
    def Type(self):
        return FCFrameType.FrameCtrl

# 采样请求帧
class FCReqFrame(FCDownFrame):
    def __init__(self, frameType, interval):
        byte0_byte3 = struct.pack('>I', interval)
        byte4_byte19 = b'\xA5' * 16
        bytesVal = byte0_byte3 + byte4_byte19
        data = struct.unpack(gDataUnpackStr, bytesVal)
        super(FCReqFrame, self).__init__(frameType = frameType, frameData = data)

#####################################################################################################################
# 具体类
class FCCtrlStopFrame(FCCtrlFrame):
    def __init__(self):
        # 基类默认参数实现赋值
        super(FCCtrlStopFrame, self).__init__()

class FCCtrlStartFrame(FCCtrlFrame):
    def __init__(self, accelerator = 0): 
        t_b = b'\x01'
        accelerator_bytes = struct.pack('>h', accelerator)
        super(FCCtrlStartFrame, self).__init__(type_byte = t_b, data = accelerator_bytes)

class FCReqTimeAcceleratorDmpQuatFrame(FCReqFrame):
    def __init__(self, interval = 100):
        super(FCReqTimeAcceleratorDmpQuatFrame, self).__init__(frameType = self.Type(), interval = interval)
    def Type(self):
        return FCFrameType.FrameReqTimeAcceleratorDmpQuat

class FCReqTimeAcceleratorEulerPid(FCReqFrame):
    def __init__(self, interval = 100):
        super(FCReqTimeAcceleratorEulerPid, self).__init__(frameType = self.Type(), interval = interval)
    def Type(self):
        return FCFrameType.FrameReqTimeAcceleratorEulerPid

class FCPidSetFrame(FCDownFrame):
    def __init__(self, euler_str, pidTuple):
        if '俯仰PID' == euler_str:
            byte0_val = 0
        elif '横滚PID' == euler_str:
            byte0_val = 1
        elif '偏航PID' == euler_str:
            byte0_val = 2
        else:
            print("FCPidSetFrame欧拉角类型错误")
            return

        byte0 = struct.pack('>b', byte0_val)
        p_bytes = struct.pack('>f', pidTuple[0])
        i_bytes = struct.pack('>f', pidTuple[1])
        d_bytes = struct.pack('>f', pidTuple[2])
        byte13_byte19 = b'\xA5' * 7
        bytesVal = byte0 + p_bytes + i_bytes + d_bytes + byte13_byte19
        data = struct.unpack(gDataUnpackStr, bytesVal)
        super(FCPidSetFrame, self).__init__(frameType = self.Type(), frameData = data)

    def Type(self):
        return FCFrameType.FramePidSet

if __name__ == '__main__':
    print("偷懒,先不写单元测试")

