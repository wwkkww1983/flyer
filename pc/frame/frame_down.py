#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from frame.frame_type import *
from frame.frame_base import *

gDataUnpackStr = '>' + 'I' * 5

#####################################################################################################################
# 抽象类
# 下行帧 上位机构造
class FCDownFrame(FCBaseFrame):
    # 抽象类 用户不使用
    def __init__(self, frameType, frameData, dataLen = 24):
        super(FCDownFrame, self).__init__()

        self.mType = struct.pack('>I', frameType.value)
        self.mLen = struct.pack('>I', dataLen)
        self.mData = struct.pack(gDataUnpackStr, *frameData)

        # 生产CRC32
        wordList = self.GetCrc32WordList()
        crc32 = FCBaseFrame.CalStm32Crc32(wordList)
        self.mCrc32 = struct.pack('>I', crc32)

# 控制帧
class FCFlyerCtrlFrame(FCDownFrame):
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
        super(FCFlyerCtrlFrame, self).__init__(frameType = FCFrameType.FrameFlyerCtrl, frameData = data)
    def Type(self):
        return FCFrameType.FrameFlyerCtrl

# 采样请求帧
class FCRequestFrame(FCDownFrame):
    def __init__(self, frameType, interval):
        byte0_byte3 = struct.pack('>I', interval)
        byte4_byte19 = b'\xA5' * 16
        bytesVal = byte0_byte3 + byte4_byte19
        data = struct.unpack(gDataUnpackStr, bytesVal)
        super(FCRequestFrame, self).__init__(frameType = frameType, frameData = data)

#####################################################################################################################
# 具体类
class FCStopFrame(FCFlyerCtrlFrame):
    def __init__(self):
        # 基类默认参数实现赋值
        super(FCStopFrame, self).__init__()

class FCStartFrame(FCFlyerCtrlFrame):
    def __init__(self, accelerator = 0): 
        t_b = b'\x01'
        accelerator_bytes = struct.pack('>h', accelerator)
        super(FCStartFrame, self).__init__(type_byte = t_b, data = accelerator_bytes)

class FCRequestTimeAcceleratorDmpQuatFrame(FCRequestFrame):
    def __init__(self, interval = 100):
        super(FCRequestTimeAcceleratorDmpQuatFrame, self).__init__(frameType = FCFrameType.FrameRequestTimeAcceleratorDmpQuat, interval = interval)
    def Type(self):
        return FCFrameType.FrameRequestTimeAcceleratorDmpQuat

class FCRequestTimeAcceleratorEulerPid(FCRequestFrame):
    def __init__(self, interval = 100):
        super(FCRequestTimeAcceleratorEulerPid, self).__init__(frameType = FCFrameType.FrameRequestTimeAcceleratorEulerPid, interval = interval)
    def Type(self):
        return FCFrameType.FrameRequestTimeAcceleratorEulerPid

if __name__ == '__main__':
    print("偷懒,先不写单元测试")

