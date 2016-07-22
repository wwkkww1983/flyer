#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import threading
from time import sleep
from serial import Serial
from serial.tools import list_ports

from PyQt5.QtWidgets import *
from PyQt5.QtCore import pyqtSignal
from PyQt5.uic import loadUiType, loadUi

from fc_waveWidget import FCWaveWidget
from fc_frame import FCFrame

FCWindowUIClass = loadUiType("fc_captureWidget.ui")

class FCCaptureWidget(QWidget):
    def __init__(self):
        super(FCCaptureWidget, self).__init__() 
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
        self.mTypeComboBox.addItem('串口')
        self.mTypeComboBox.addItem('WiFi')
        self.mTypeComboBox.currentIndexChanged.connect(self.ChangeCommType) 
        allSerial = list_ports.comports()
        for ser in allSerial: 
            self.mComNameComboBox.addItem(ser[0])
        self.mBuadLineEdit.setText("115200")
        self.mIntervalLineEdit.setText("10")
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

    def StartCapture(self):
        # step1: 启动串口线程
        self.mSerial = Serial()
        comPort = self.mComNameComboBox.currentText()
        comBaudrate = self.mBuadLineEdit.text()
        self.mSerial.port = comPort
        self.mSerial.baudrate = comBaudrate
        self.mSerial.timeout = 0 #非阻塞调用
        self.mSerial.open()
        self.mRecvThread = threading.Thread(target=self.RecvFunc)
        self.mRecvThread.daemon = True # 主线程结束 子线程也结束
        self.mRecvThread.start()

        # step2: 组请求帧
        # TODO: 需要根据控制界面定制
        time = int(self.mIntervalLineEdit.text())
        #print(time)
        frameType = b'\x20\x00\x00\x03'
        frameLen = 16
        frameData = time

        frame = FCFrame(frameType, frameLen, frameData)
        buf = frame.GetBytes()
        print(buf)

        # step3: 发帧
        self.mSerial.write(buf)

        print("开始采集")

    def StopCapture(self):
        # step1: 组停止帧

        # step2: 发帧

        # step3: 停止串口线程
        self.mRecvThread.join(10) #等待10s
        self.mSerial.close()
        self.mSerial = None
        self.mRecvThread = None
        print("停止采集")

    def RecvFunc(self):
        line_nums = 0;
        #line_bytes = ser.read(g_frame_size)
        while self.mCapturing:
            #line = self.mSerial.readline()
            line = self.mSerial.read(1)
            if 0 != len(line): 
                line_nums = line_nums + 1 
                line_str = line
                #line_str = line.decode('utf8') 
                print(line_nums, end = ':')
                print(line_str)

    def ChangeCommType(self, typeIndex):
        if 0 != typeIndex: 
            msgBox = QMessageBox();
            msgBox.setText("暂时只实现串口.\n")
            msgBox.exec_();
            self.mTypeComboBox.setCurrentIndex(0)

if __name__ == '__main__': 
    app = QApplication(sys.argv)
    win = FCCaptureWidget()
    win.show()
    sys.exit(app.exec_())

