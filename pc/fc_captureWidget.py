#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct
import socket
import threading
from time import sleep

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.uic import loadUiType, loadUi

from fc_waveWidget import FCWaveWidget
from fc_frame import FCRequestTimeAndDmpQuatFrame
from fc_frame import FCUpFrame
from fc_frame import FCFrameType
from fc_serial import FCSerial
from fc_net import FCUdp

FCWindowUIClass = loadUiType("fc_captureWidget.ui")

class FCCaptureWidget(QWidget): 
    sAppendConsole = pyqtSignal(str, name='sAppendConsole')
    sUpdateQuat = pyqtSignal((str, str, str, str, str), name='sUpdateQuat')

    def __init__(self):
        super(FCCaptureWidget, self).__init__() 
        
        # 通信线程&串口&标记
        self.mCapturing = False
        self.mComm = None
        self.mCommType = '网络' # 默认使用网络

        # 初始化UI
        self.mUi = FCWindowUIClass[0]()
        self.mUi.setupUi(self)

        # 初始化控制控件
        self.mCommGroupBox = self.mUi.commGroupBox
        self.mDataGroupBox = self.mUi.dataGroupBox
        self.mTypeComboBox = self.mUi.typeComboBox
        self.mComNameComboBox = self.mUi.comNameComboBox
        self.mBuadLineEdit = self.mUi.buadLineEdit
        self.mIntervalLineEdit = self.mUi.intervalLineEdit
        self.mDmpQuatCheckBox = self.mUi.dmpQuatCheckBox
        self.mCapturePushButton = self.mUi.capturePushButton
        self.mConsolePlainTextEdit = self.mUi.consolePlainTextEdit
        self.sAppendConsole.connect(self.AppendConsole)
        self.mRunTimeLabel = self.mUi.runTimeLabel
        self.mDmpQuatLabel = self.mUi.dmpQuatLabel
        self.mThetaLabel = self.mUi.thetaLabel
        self.mPhiLabel = self.mUi.phiLabel
        self.mPsiLabel = self.mUi.psiLabel
        self.sUpdateQuat.connect(self.UpdateQuat)
        self.mIpLabel = self.mUi.ipLabel
        localIP = socket.gethostbyname(socket.gethostname()) # 获取本地IP
        localIPStr = ("%s" % localIP)
        self.mIpLabel.setText(localIPStr)
        self.mPortLineEdit = self.mUi.portLineEdit 
        self.mNetGroupBox = self.mUi.netGroupBox
        self.mSerGroupBox = self.mUi.serGroupBox
        self.mTypeComboBox.addItem('网络') # 0
        self.mTypeComboBox.addItem('串口') # 1 
        self.mTypeComboBox.currentIndexChanged.connect(self.ChangeCommType)
        allPortsName = FCSerial.ListAllPorts()
        for portName in allPortsName:
            self.mComNameComboBox.addItem(portName)
        self.mBuadLineEdit.setText("115200")
        self.mIntervalLineEdit.setText("100")
        self.mDmpQuatCheckBox.setChecked(True)
        self.mCapturePushButton.clicked.connect(self.ChangeState)

        # 加入波形控件
        self.mWaveWidget = FCWaveWidget()
        self.mWaveGroupBox = self.mUi.waveGroupBox
        vbox = QVBoxLayout()
        vbox.addWidget(self.mWaveWidget)
        self.mWaveGroupBox.setLayout(vbox)

        # 上行帧 表驱动 字典 
        self.updateFuncDict = {
                FCFrameType.FrameDataTimeAndDmpQuat: self.UpdateTimeAndDmpQuat,
                FCFrameType.FramePrintText : self.UpdatePrintText,
                FCFrameType.FrameError : self.UpdateErrorFrame,
            }

        self.commDict = {
                #键   : (类,        通信参数)
                '网络': (FCUdp,     (self.mIpLabel.text(), int(self.mPortLineEdit.text()))),
                '串口': (FCSerial,  (self.mComNameComboBox.currentText(), self.mBuadLineEdit.text())),
                }


    def ChangeState(self, checked):
        if self.mCapturing:
            self.mCapturing = False
            self.StopCapture()
            self.mCapturePushButton.setText("开始采集")
            self.mCommGroupBox.setEnabled(True)
            self.mDataGroupBox.setEnabled(True)
        else:
            self.mCapturing = True
            self.StartCapture()
            self.mCapturePushButton.setText("停止采集")
            self.mCommGroupBox.setEnabled(False)
            self.mDataGroupBox.setEnabled(False)

    def ChangeCommType(self, typeIndex):
        if 0 == typeIndex: # 网络
            self.mCommType = "网络"
            self.mNetGroupBox.setEnabled(True)
            self.mSerGroupBox.setEnabled(False)
            return
        if 1 == typeIndex: # 串口
            self.mCommType = "串口"
            self.mSerGroupBox.setEnabled(True)
            self.mNetGroupBox.setEnabled(False)
            return

    def StartCapture(self):
        # 构造通信链路
        commClass = self.commDict[self.mCommType][0]
        paras = self.commDict[self.mCommType][1]
        print(paras)
        self.mComm = commClass(*paras)

        # step2: 组请求帧
        time = int(self.mIntervalLineEdit.text())
        #print(time)

        frame = FCRequestTimeAndDmpQuatFrame(time)
        buf = frame.GetBytes()
        print("发送下行帧" , end = ':')
        print(paras)
        frame.Print()

        # step3: 发帧
        self.mComm.Write(buf)
        print("开始采集")

    def StopCapture(self):
        # step1: 组停止帧

        # step2: 发帧

        # step3: 停止串口线程
        self.mComm.Close()
        self.mComm = None
        print("停止采集")

    def RecvFunc(self):
        # 接收帧头  type + len = 8Bytes
        frameHeadLen = 8
        # 计算循环使用的常量
        while self.mCapturing:
            # 获取type+len
            frameHead = self.mComm.read(frameHeadLen)
            #FCUpFrame.PrintBytes(frameHead) 
            
            # 获取data+crc32
            frameDataAndCrc32Len = FCUpFrame.ParseLen(frameHead)
            #print(frameDataAndCrc32Len)
            #FCUpFrame.PrintBytes(frameHead)
            frameDataAndrCrc32 = self.mComm.read(frameDataAndCrc32Len)
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

    def UpdateTimeAndDmpQuat(self, frame):
        time = frame.GetTime()
        dmpQuat = frame.GetGmpQuat()
        euler = dmpQuat.ToEuler()

        timeText = "运行:%7.1fs" % (time / 1000.0)
        dmpQuatText = dmpQuat.ToString()
        thetaText = "俯仰角:%+04.2fd" % euler.Theta()
        phiText   = "横滚角:%+04.2fd" % euler.Phi()
        psiText   = "偏航角:%+04.2fd" % euler.Psi()

        self.sUpdateQuat.emit(timeText, dmpQuatText, thetaText, phiText, psiText)

    def UpdatePrintText(self, frame):
        #print("接收文本帧(%s:%s):" % (self.mComm.port, self.mComm.baudrate))
        #frame.Print()
        text = frame.GetText()
        #print(1)
        #print(text)
        #print(2)
        #self.mConsolePlainTextEdit.appendPlainText(text) 
        self.sAppendConsole.emit(text)

    def AppendConsole(self, text):
        # 有额外的换行
        #self.mConsolePlainTextEdit.appendPlainText(text)
        self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
        self.mConsolePlainTextEdit.insertPlainText(text)
        self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)

    def UpdateQuat(self, timeText, dmpQuatText, thetaText, phiText, psiText):
        self.mRunTimeLabel.setText(timeText)
        self.mDmpQuatLabel.setText(dmpQuatText)
        self.mThetaLabel.setText(thetaText)
        self.mPhiLabel.setText(phiText)
        self.mPsiLabel.setText(psiText)

    def UpdateErrorFrame(self, frame):
        return
        print("接收错误帧(%s:%s):" % (self.mComm.port, self.mComm.baudrate))
        frame.Print()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = FCCaptureWidget()
    win.show()
    sys.exit(app.exec_())

