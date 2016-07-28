#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct
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

FCWindowUIClass = loadUiType("fc_captureWidget.ui")

class FCCaptureWidget(QWidget): 
    def __init__(self):
        super(FCCaptureWidget, self).__init__() 
        
        # 上行帧 表驱动 字典 
        self.updateFuncDict = {
                FCFrameType.FrameRequestTimeAndDmpQuat : self.UpdateTimeAndDmpQuat,
                FCFrameType.FramePrintText : self.UpdatePrintText,
                FCFrameType.FrameError : self.UpdateErrorFrame,
            }

        # 通信线程&串口&标记
        self.mCapturing = False
        self.mSerial = None
        self.mRecvThread = None

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
        self.mRunTimeLabel = self.mUi.runTimeLabel
        self.mDmpQuatLabel = self.mUi.dmpQuatLabel
        self.mThetaLabel = self.mUi.thetaLabel
        self.mphiLabel = self.mUi.phiLabel
        self.mpsiLabel = self.mUi.psiLabel
        self.mTypeComboBox.addItem('串口')
        self.mTypeComboBox.addItem('WiFi')
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
        if 0 != typeIndex:
            msgBox = QMessageBox();
            msgBox.setText("暂时只实现串口.\n")
            msgBox.exec_();
            self.mTypeComboBox.setCurrentIndex(0)

    def StartCapture(self):
        # TODO: 需要根据控制界面定制
        # step1: 启动串口线程
        self.mSerial = FCSerial()
        comPort = self.mComNameComboBox.currentText()
        comBaudrate = self.mBuadLineEdit.text()
        self.mSerial.port = comPort
        self.mSerial.baudrate = comBaudrate
        self.mSerial.timeout = None # 阻塞调用
        self.mSerial.open()
        self.mSerial.reset_input_buffer() # 复位缓存
        self.mRecvThread = threading.Thread(target=self.RecvFunc)
        self.mRecvThread.daemon = True # 主线程结束 子线程也结束
        self.mRecvThread.start()

        # step2: 组请求帧
        time = int(self.mIntervalLineEdit.text())
        #print(time)

        frame = FCRequestTimeAndDmpQuatFrame(time)
        buf = frame.GetBytes()
        print("发送下行帧(%s:%s):" % (self.mSerial.port, self.mSerial.baudrate))
        frame.Print()

        # step3: 发帧
        self.mSerial.write(buf)

        print("开始采集")

    def StopCapture(self):
        # step1: 组停止帧

        # step2: 发帧

        # step3: 停止串口线程
        self.mRecvThread.join(1) #等待0s
        self.mSerial.close()
        self.mSerial = None
        self.mRecvThread = None
        print("停止采集")

    def RecvFunc(self):
        # 接收帧头  type + len = 8Bytes
        # 计算循环使用的常量
        frameHeadLen = 8
        while self.mCapturing:
            # 获取type+len
            frameHead = self.mSerial.ReadWithTimeout(frameHeadLen)
            if frameHeadLen == len(frameHead):
                #FCUpFrame.PrintBytes(frameHead)

                # 获取data+crc32
                frameDataAndCrc32Len = FCUpFrame.ParseLen(frameHead)
                frameDataAndrCrc32 = self.mSerial.ReadWithTimeout(frameDataAndCrc32Len)
                buf = frameHead + frameDataAndrCrc32

                # 解析帧
                frame = FCUpFrame.Parse(buf)

                # 使用帧 更新界面
                self.UpdateByNewFrame(frame)

    def UpdateByNewFrame(self, frame): 
        frameType = frame.Type()

        # 表驱动
        updateFunc = self.updateFuncDict[frameType]
        self.updateFunc(frame)
        #self.update()

    def UpdateTimeAndDmpQuat(self, frame):
        print("UpdateTimeAndDmpQuat")
        time = frame.GetTime()
        dmpQuat = frame.GetGmpQuat()
        euler = dmpQuat.ToEuler()

        timeText = "运行:%7.2s" % time / 1000.0
        self.mRunTimeLabel.setText(timeText)

        dmpQuatText = dmpQuat.ToString()
        self.mDmpQuatLabel.setText(dmpQuatText)

        thetaText = "俯仰角:%+4.1s" % euler.Theta()
        phiText   = "横滚角:%+4.1s" % euler.Phi()
        psiText   = "偏航角:%+4.1s" % euler.Psi()
        self.mThetaLabel.setText(thetaText)
        self.mphiLabel.setText(phiText)
        self.mpsiLabel.setText(psiText)

    def UpdatePrintText(self, frame):
        print("UpdatePrintText")
        text = frame.GetText()
        self.mConsolePlainTextEdit.appendPlainText(text)

    def UpdateErrorFrame(self, frame):
        print("接收错误帧(%s:%s):" % (self.mSerial.port, self.mSerial.baudrate))
        frame.Print()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = FCCaptureWidget()
    win.show()
    sys.exit(app.exec_())

