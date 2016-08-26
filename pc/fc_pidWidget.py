#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import socket
import threading
from datetime import datetime

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from PyQt5.uic import loadUiType, loadUi

from fc_net import FCUdp
from fc_waveWidget import FCWaveWidget

# 帧类型枚举
from fc_frame import FCFrameType
# 上行帧基类(用于解析帧长)
from fc_frame import FCUpFrame

from fc_frame import FCRequestTimeAcceleratorEulerPid

class FCPidWidget(QWidget): 
    sAppendConsole = pyqtSignal(str, name='sAppendConsole')
    #sUpdateAcceleratorQuat = pyqtSignal((str, str, str, str, str, int, int, int, int, int), name='sUpdateAcceleratorQuat') 

    def __init__(self, uiFile):
        super(FCPidWidget, self).__init__() 
        # 上行帧 表驱动 字典 
        self.updateFuncDict = {
                #FCFrameType.FrameDataTimeAcceleratorDmpQuat: self.UpdateTimeAcceleratorDmpQuat,
                FCFrameType.FrameDataTimeAcceleratorEulerPid: self.UpdateTimeAcceleratorEulerPid,
                FCFrameType.FramePrintText : self.UpdatePrintText,
                FCFrameType.FrameError : self.UpdateErrorFrame,
            }

        # 采集数据文件
        self.mDataFile = None
        # 通信线程&线程标记
        self.mCapturing = False
        self.mComm = None
        self.mRecvThread = None
        self.mNewFlyerInitDoneStr = '初始化完成,进入主循环.'
        
        # 初始化UI
        FCWindowUIClass = loadUiType(uiFile)
        self.mUi = FCWindowUIClass[0]()
        self.mUi.setupUi(self) 

        # ip控件
        self.mIpLabel = self.mUi.ipLabel
        localIP = socket.gethostbyname(socket.gethostname()) # 获取本地IP
        localIPStr = ("%s" % localIP)
        self.mIpLabel.setText(localIPStr)
        # 端口控件
        self.mPortLineEdit = self.mUi.portLineEdit 
        # 采样间隔
        self.mIntervalLineEdit = self.mUi.intervalLineEdit

        # 按钮
        self.mCapturePushButton = self.mUi.capturePushButton
        self.mCapturePushButton.clicked.connect(self.ChangeState)

        # 加入波形控件
        self.mWaveWidget = FCWaveWidget()
        self.mWaveGroupBox = self.mUi.waveGroupBox
        vbox = QVBoxLayout()
        vbox.addWidget(self.mWaveWidget)
        self.mWaveGroupBox.setLayout(vbox)

        # 本地保存文件相关控件
        self.mSaveLineEdit = self.mUi.saveLineEdit 

        # 可打印帧显示控件
        self.mConsolePlainTextEdit = self.mUi.consolePlainTextEdit
        
    def ChangeState(self, checked):
        if self.mCapturing: # 停止采集
            # 关闭保存的文件
            dataFilePath = self.mSaveLineEdit.text()
            self.mDataFile.close()

            self.mCapturing = False
            self.mCapturePushButton.setText("开始采集")
            self.StopCapture()
        else: # 开始采集
            # 打开保存的文件
            dirPath = self.mSaveLineEdit.text()
            fileName = datetime.now().strftime('%Y_%m_%d_%H_%M_%S') + ".dat"
            fullName = os.path.join(dirPath, fileName)
            self.mDataFile = open(fullName, mode = 'bw')
            #self.mDataFile.write("数据如下:\n".encode('utf-8'))

            self.mCapturing = True
            self.mCapturePushButton.setText("停止采集")
            self.StartCapture() 

    def StartCapture(self):
        # step1: 构造通信链路
        commClass = FCUdp
        paras = (self.mIpLabel.text(), int(self.mPortLineEdit.text()))
        self.mComm = commClass(*paras)
        #print(paras)

        # step2: 获取下位机握手(有阻塞,所以使用后台线)
        self.mRecvThread = threading.Thread(target=self._RecvFunc)
        self.mRecvThread.start()

    def StopCapture(self):
        if not self.mComm:
            return
        # step1: 组停止帧

        # step2: 发帧

        # step3: 停止串口线程
        self.mComm.Close()
        self.mComm = None
        print("停止监控")

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
            
            # 解析帧
            frame = FCUpFrame.Parse(buf) 
            
            # 使用帧 更新界面
            self.UpdateByNewFrame(frame)

    def UpdateByNewFrame(self, frame): 
        frameType = frame.Type()

        # 表驱动
        updateFunc = self.updateFuncDict[frameType]
        updateFunc(frame)
        #self.update()

    def UpdateTimeAcceleratorEulerPid(self, frame):
        time = frame.GetTime()
        accelerator = frame.GetAccelrator()
        euler = frame.GetEuler()
        pid = frame.GetPid()

        # 加入数据
        #self.mWaveWidget.Append(time, euler, accelerator)

        # 保存到文件
        self.mDataFile.write(frame.GetBytes())

        #self.sUpdateAcceleratorEulerPid.emit(timeText, accelerator euler, pid)

    def UpdatePrintText(self, frame):
        #print("接收文本帧:")
        #frame.Print()
        text = frame.GetText()
        #print(1)
        #print(text)
        #print(2)
        #self.mConsolePlainTextEdit.appendPlainText(text) 

        #print(text)
        #print(self.mNewFlyerInitDoneStr in text)
        # 飞控初始化完成 发送命令
        if self.mNewFlyerInitDoneStr in text:
            self.SendRequestCaptureDataCmd()

        self.sAppendConsole.emit(text)

    def AppendConsole(self, text):
        # 有额外的换行
        #self.mConsolePlainTextEdit.appendPlainText(text)
        self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
        self.mConsolePlainTextEdit.insertPlainText(text)
        self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)

    def UpdateErrorFrame(self, frame):
        return
        print("接收错误帧(%s:%s):" % (self.mComm.port, self.mComm.baudrate))
        frame.Print()

    def SendRequestCaptureDataCmd(self):
        # TODO:由界面定制
        interval = int(self.mIntervalLineEdit.text())
        #print(interval) 

        frame = FCRequestTimeAcceleratorEulerPid(interval)
        buf = frame.GetBytes()
        print("采样数据请求帧" , end = ':')
        frame.Print()

        self.mComm.Write(buf)
        print("开始监控")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = FCAnalysisWidget()
    win.show()
    sys.exit(app.exec_())

