#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct

from enum import Enum

from algo.quat import FCQuat

gFillByte = 165 # b'\xa5' 

class FCFrameType(Enum):
    # 以下类型基本帧类型(用户不使用)
    _Up                 = 0x80000000
    _Down               = 0x00000000
    FrameFlyerCtrl      = 0x40000000
    _Capture            = 0x20000000
    _Text               = 0x10000000

    _Pid                = 0x00000100
    _Euler              = 0x00000080
    _Accelerator        = 0x00000040
    _DataPress          = 0x00000020
    _DataCompass        = 0x00000010
    _DataGyro           = 0x00000008
    _DataAccel          = 0x00000004
    _DataDmpQuat        = 0x00000002
    _DataTime           = 0x00000001

    # 以下类型用户使用
    # dmp四元数采集请求帧
    FrameRequestTimeAcceleratorDmpQuat  = _Down | _Capture | _Accelerator | _DataDmpQuat | _DataTime
    FrameRequestTimeAcceleratorEulerPid = _Down | _Capture | _Pid | _Euler | _Accelerator | _DataTime

    # dmp四元数采集数据帧
    FrameDataTimeAcceleratorDmpQuat  = _Up | _Capture | _Accelerator | _DataDmpQuat | _DataTime
    FrameDataTimeAcceleratorEulerPid = _Up | _Capture | _Pid | _Euler | _Accelerator | _DataTime
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
            print("错误帧如下:%d." % dataLen)
            self.Print()
            while True:
                pass
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

    def GetBytes(self):
        buf = self.mType + self.mLen + self.mData + self.mCrc32
        return buf

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

        if dataLen < 8: # 小于4 则无数据
            print("下行帧构造失败:数据长错误,为:%d.\n" % dataLen)

        self.mType = struct.pack('>I', frameType.value)
        self.mLen = struct.pack('>I', dataLen)
        self.mData = struct.pack('>I', frameData)

        # 生产CRC32
        wordList = self.GetCrc32WordList()
        crc32 = _FCBaseFrame.CalStm32Crc32(wordList)
        self.mCrc32 = struct.pack('>I', crc32)

class _FCFlyerCtrlFrame(_FCDownFrame):
    def __init__(self, type_byte = b'\x00', data = b'\xA5\xA5'):
        # byte0 : 0(加速)
        # val   : byte1 << 8 | byte2
        # byte3 : 0xA5
        byte0 = type_byte
        byte2_byte3 = data
        byte3 = b'\xA5'
        bytesVal = byte0 + byte2_byte3 + byte3
        #print(bytesVal)
        data = struct.unpack('>I', bytesVal)[0]
        #print(data)
        super(_FCFlyerCtrlFrame, self).__init__(frameType = FCFrameType.FrameFlyerCtrl,
                frameData = data)
    def Type(self):
        return FCFrameType.FrameFlyerCtrl

class FCStopFrame(_FCFlyerCtrlFrame):
    def __init__(self):
        # 基类默认参数实现赋值
        super(FCStopFrame, self).__init__()

class FCStartFrame(_FCFlyerCtrlFrame):
    def __init__(self, accelerator = 0): 
        t_b = b'\x01'
        accelerator_bytes = struct.pack('>h', accelerator)
        super(FCStartFrame, self).__init__(type_byte = t_b, data = accelerator_bytes)

# 上行帧 上位机解析
class FCUpFrame(_FCBaseFrame): 
    def __init__(self, frameBuf):
        super(FCUpFrame, self).__init__()

        frameLen = len(frameBuf) 
        #print(frameLen)
        #_FCBaseFrame.PrintBytes(frameBuf)
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

class FCPrintTextFrame(FCUpFrame):
    def __init__(self, frameBuf): 
        super(FCPrintTextFrame, self).__init__(frameBuf)

    def Type(self): 
        return FCFrameType.FramePrintText

    def GetText(self):
        # 清理末尾的填充
        textBuf = self.mData
        i = -1

        while gFillByte == textBuf[i]:
            i = i - 1
        # 仅处理有尾部填充的情况
        if i < -1:
            textBuf = textBuf[:i+1]

        # 生成字符串 
        text = textBuf.decode('utf8')
        return text

class FCRequestTimeAcceleratorDmpQuatFrame(_FCDownFrame):
    def __init__(self, interval = 100):
        super(FCRequestTimeAcceleratorDmpQuatFrame, self).__init__(frameType = FCFrameType.FrameRequestTimeAcceleratorDmpQuat, frameData = interval)
    def Type(self):
        return FCFrameType.FrameRequestTimeAcceleratorDmpQuat

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

class FCRequestTimeAcceleratorEulerPid(_FCDownFrame):
    def __init__(self, interval = 100):
        super(FCRequestTimeAcceleratorEulerPid, self).__init__(frameType = FCFrameType.FrameRequestTimeAcceleratorEulerPid, frameData = interval)
    def Type(self):
        return FCFrameType.FrameRequestTimeAcceleratorEulerPid

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

