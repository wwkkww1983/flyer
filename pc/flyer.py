#!/usr/bin/env python3

import time
import math
import struct
import random
from socket import *

# 帧
from frame.base import FCBaseFrame
from frame.base import gFillByte
from frame.up import FCUpFrame
from frame.up import gTimeLen

# 数据类
from frame.data.quat import FCQuat
from frame.data.euler import FCEuler
from frame.data.accelerator import FCAccelerator
from frame.data.pid import FCPid

# 配置
from config import gLocalIP
from config import gLocalPort

from config import gRad2Arc

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

        dmpQuat = frameDict['DMP四元数']
        accel = frameDict['加计']
        gyro = frameDict['陀螺仪']
        compass = frameDict['磁计']
        press = frameDict['压力']
        accelerator = frameDict['油门']
        euler = frameDict['欧拉角']
        pid = frameDict['PID']

        # 时间
        now = Flyer.Now()
        data = struct.pack('>I', now)

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

        if gTimeLen == len(data):
            print('数据帧不可以仅有时间')
            return None

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
        print('[%05d]:' % now, end = '')
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
        print('[%05d]:%s' % (now, printfStr), end = '')


if __name__ == '__main__': 
    flyer = Flyer()
    time.sleep(0.01) # 10 ms

    # 发送字符串
    flyer.SendPrintFrame('世界，您好！abc,123\r\n') 
    flyer.SendPrintFrame('开始启动了.\r\n') 

    interval = 0.00001 # 1s
    timeMax = 600 # 60s
    for t in range(0, timeMax): 
        euler_max = gRad2Arc * math.pi / 180
        euler_theta_rand = random.uniform(- euler_max * 10, euler_max * 10)
        pid_theta_rand =  random.uniform(-20, 20)
        euler = FCEuler(euler_theta_rand, 0, 0)
        pid = FCPid(pid_theta_rand, 0, 0)
        frameDict = {'DMP四元数': None, '加计': None, '陀螺仪': None, '磁计': None, '压力': None, '油门': None, 'PID': pid, '欧拉角': euler,}
        flyer.SendDataFrame(frameDict)
        time.sleep(interval) # 延迟不精确

