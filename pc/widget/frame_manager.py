#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import socket
import threading

# 配置
from config import gLocalIP
from config import gLocalPort
from config import gSaveDataFileFullName

# 通信
from comm import FCUdp

# 协议帧
from frame.up import FCUpFrame

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *

class FCFrameManager(QWidget):
    """
    1. 发送下行帧
    2. 接收上行帧

    注意: 不实现GUI逻辑
    """
    sRecvNewUpFrame = pyqtSignal(FCUpFrame, name = 'sRecvNewUpFrame')

    def __init__(self):
        super(FCFrameManager, self).__init__() 
        
        # 标识flyer正常启动
        self.mNewFlyerInitDoneStr = '初始化完成,进入主循环.'

        # 采集数据文件
        self.mDataFile = None

        # 通信线程&线程标记
        self.mCapturing = False
        self.mComm = None
        self.mRecvThread = None

        # 启动自动保存和上行帧监控
        self.StartSave()
        self.StartMonitor()

    def closeEvent(self, event):
        self.StopMonitor()
        self.StopSave()

    def StartSave(self): 
        # 新建目录
        save_data_file_path = os.path.dirname(gSaveDataFileFullName)
        if not os.path.exists(save_data_file_path):
            os.mkdir(save_data_file_path)

        self.mDataFile = open(gSaveDataFileFullName, mode = 'bw')
        print("开始记录与:%s" % gSaveDataFileFullName)

    def StopSave(self): 
        self.mDataFile.close()
        print("停止记录")

    def StopMonitor(self):
        # 关闭后台接收线程
        self.mRecvThread.join(0.2) # 等待200ms 需要与recv超时值配合
        self.mComm.Close()
        self.mComm = None
        self.mCapturing = False
        print("停止监控")

    def StartMonitor(self):
        # step1: 构造通信链路
        commClass = FCUdp
        paras = (gLocalIP, gLocalPort)
        self.mComm = commClass(*paras)

        # step2: 获取下位机握手(有阻塞,所以使用后台线)
        self.mRecvThread = threading.Thread(target=self._RecvFunc)
        self.mRecvThread.daemon = True # 主线程结束 子线程也结束
        self.mCapturing = True
        self.mRecvThread.start()

        print(paras, end = '')
        print("开始监控")

    def _RecvFunc(self):
        # 接收帧头  type + len = 8Bytes
        frameHeadLen = 8
        # 计算循环使用的常量
        while self.mCapturing:
            # 获取type+len
            frameHead = self.mComm.Read(frameHeadLen)
            # 未获取到有效数据
            if not frameHead:
                continue
            #print(frameHead)
            #FCUpFrame.PrintBytes(frameHead) 
            
            # 获取data+crc32
            frameDataAndCrc32Len = FCUpFrame.ParseLen(frameHead)
            #print(frameDataAndCrc32Len)
            #FCUpFrame.PrintBytes(frameHead)
            frameDataAndrCrc32 = self.mComm.Read(frameDataAndCrc32Len)
            buf = frameHead + frameDataAndrCrc32 

            # 记录 
            self.mDataFile.write(buf)
            
            # 构造上行帧
            frame = FCUpFrame(buf) 
            self.sRecvNewUpFrame.emit(frame)

    def SendDownFrame(self, downFrame):
        buf = downFrame.GetBytes()
        self.mComm.Write(buf) 
        downFrame.Print()

if __name__ == '__main__':
    manager = FCFrameManager()

