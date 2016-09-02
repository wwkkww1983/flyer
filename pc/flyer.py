#!/usr/bin/env python3

import struct
import time
from socket import *

# 帧
from frame.base import FCBaseFrame
from frame.base import gFillByte
from frame.up import FCUpFrame

# 数据类
from frame.data.quat import FCQuat

# 配置
from config import gLocalIP
from config import gLocalPort

# 类型
from frame.type import FCFrameType

gFcAddr = (gLocalIP, gLocalPort)
gCrcLen = 4
gFillUint = 4
gStartTime = time.perf_counter() 

class Flyer():
    def __init__(self):
        self.mUdpSocket = socket(AF_INET, SOCK_DGRAM)

    def __del__(self):
        self.mUdpSocket.close()

    @staticmethod
    def Now():
        now = time.perf_counter() - gStartTime
        nowInt = int(now * 5000) & 0xffffffff # 5000表示精度为 1/5000 s
        return nowInt

    @staticmethod
    def AppendFillBytes(data):
        # 无需填充
        left = len(data) % gFillUint
        if 0 == left:
            return data
        else: 
            fillLength = gFillUint - left

        return data + gFillByte * fillLength

    def Send(self, data): 
        print("send to %s:%d:" % gFcAddr, end = '')
        self.mUdpSocket.sendto(data, gFcAddr)

    def SendDataFrame(self, frameDict):
        frameType = FCFrameType.FrameUp.value | FCFrameType.FrameReq.value
        frameLen = 0;
        frameCrc32 = 0;
        data = b''

        time = frameDict['time']
        text = frameDict['text']
        dmpQuat = frameDict['dmpQuat']
        accel = frameDict['accel']
        gyro = frameDict['gyro']
        compass = frameDict['compass']
        press = frameDict['press']
        accelerator = frameDict['accelerator']
        euler = frameDict['euler']
        pid = frameDict['pid']
        if None == time:
            print('SendDataFrame没有时间数据的帧不合法')
            return None
        data = struct.pack('>I', time)

        if frameDict['text']:
            print('SendDataFrame不能发送文本帧,发送文本帧请使用SendPrintFrame')
            return None

        if dmpQuat:
            frameType |= FCFrameType.FrameDmpQuat.value
            data += dmpQuat.ToBytes() # + b'\x00' + b'\x00' + b'\x00' + b'\x00'

        if accel:
            frameType |= FCFrameType.FrameAccel.value
            data += accel.ToBytes()

        if gyro:
            frameType |= FCFrameType.FrameGyro.value
            data += gyro.ToBytes()

        if compass:
            frameType |= FCFrameType.FrameCompass.value
            data += compass.ToBytes()

        if press:
            frameType |= FCFrameType.FramePress.value
            data += press.ToBytes()

        if accelerator:
            frameType |= FCFrameType.FrameAccelerator.value
            data += accelerator.ToBytes()

        if euler:
            frameType |= FCFrameType.FrameEuler.value
            data += euler.ToBytes()

        if pid:
            frameType |= FCFrameType.FramePid.value
            data += pid.ToBytes()

        # 填充
        #FCBaseFrame.PrintBytes(data)
        #print(frameLen)
        data = Flyer.AppendFillBytes(data)
        #FCBaseFrame.PrintBytes(data)

        frameLen = len(data) + 4
        typeBytes = struct.pack('>I', frameType)
        lenBytes = struct.pack('>I', frameLen)
        dataBytes = data 
        crc32Bytes = b'\x00\x00\x00\x00'
        allBytes = typeBytes + lenBytes + dataBytes  + crc32Bytes

        # 计算crc
        frame = FCUpFrame(allBytes) 
        crc32 = frame.GetCalCrc32()

        # 构造最终帧
        allBytes = typeBytes + lenBytes + dataBytes + struct.pack('>I', crc32)
        self.Send(allBytes)
        print('[%05d],' % time, end = '')
        FCBaseFrame.PrintBytes(allBytes)

    def SendPrintFrame(self, printfStr):
        typeBytes = struct.pack('>I',  FCFrameType.FrameUpPrint.value) 

        now = Flyer.Now()
        # 1970以来的ms数
        dataBytes = struct.pack('>I', now)
        # 数据域: 字符串 + 填充
        dataBytes = dataBytes + printfStr.encode('utf8')

        # 填充
        dataBytes = Flyer.AppendFillBytes(dataBytes)

        # (字符串+填充)+crc32
        length = len(dataBytes) + gCrcLen
        lenBytes = struct.pack('>I', length) 
        
        data = typeBytes + lenBytes + dataBytes + b'\x00' * gCrcLen
        upFrame = FCUpFrame(data)
        wordList = upFrame.GetCrc32WordList()
        crc32 = FCBaseFrame.CalStm32Crc32(wordList)
        crc32Bytes = struct.pack('>I', crc32)
        data = data[0:-gCrcLen] + crc32Bytes

        upFrame = FCUpFrame(data)
        self.Send(data)
        print('[%05d],%s' % (now, printfStr), end = '')


if __name__ == '__main__': 
    flyer = Flyer()
    time.sleep(0.01) # 10 ms

    # 发送字符串
    flyer.SendPrintFrame('你好！\r\n')
    flyer.SendPrintFrame('数字:123\r\n')
    flyer.SendPrintFrame('英文:abc\r\n')
    flyer.SendPrintFrame('混搭:世界，您好！abc,123\r\n') 
    
    # 发送dmpQuat
    frameDict = {
            'time': Flyer.Now(),
            'text': None,
            'dmpQuat': FCQuat(1.0, 2.0, 3.0, 4.0),
            'accel': None,
            'gyro': None,
            'compass': None,
            'press': None,
            'accelerator': None,
            'euler': None,
            'pid': None,}
    flyer.SendDataFrame(frameDict)

