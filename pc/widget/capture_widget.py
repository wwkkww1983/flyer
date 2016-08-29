#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import struct
import socket
import threading
from datetime import datetime

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.uic import loadUiType, loadUi

from widget.wave_widget import FCWaveWidget

# 帧
from frame import *
# 通信
from comm import *

FCWindowUIClass = loadUiType("widget/ui/fc_captureWidget.ui")

class FCCaptureWidget(QWidget): 
    sAppendConsole = pyqtSignal(str, name='sAppendConsole')
    sUpdateAcceleratorQuat = pyqtSignal((str, str, str, str, str, int, int, int, int, int), name='sUpdateAcceleratorQuat')

    def __init__(self):
        super(FCCaptureWidget, self).__init__() 

        # 采集数据文件
        self.mDataFile = None

        # 通信线程&串口&标记
        self.mCapturing = False
        self.mComm = None
        self.mRecvThread = None
        self.mCommType = '网络' # 默认使用网络
        self.mNewFlyerInitDoneStr = '初始化完成,进入主循环.'

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
        self.mAcceleratorCheckBox = self.mUi.acceleratorCheckBox
        self.mCapturePushButton = self.mUi.capturePushButton
        self.mCommandPushButton = self.mUi.commandPushButton
        self.mStopPushButton = self.mUi.stopPushButton
        self.mAcceleratorSpinBox = self.mUi.acceleratorSpinBox
        self.mAcceleratorLabel = self.mUi.acceleratorLabel
        self.mConsolePlainTextEdit = self.mUi.consolePlainTextEdit
        self.sAppendConsole.connect(self.AppendConsole)
        self.mRunTimeLabel = self.mUi.runTimeLabel
        self.mDmpQuatLabel = self.mUi.dmpQuatLabel
        self.mThetaLabel = self.mUi.thetaLabel
        self.mPhiLabel = self.mUi.phiLabel
        self.mPsiLabel = self.mUi.psiLabel
        self.mFrontLabel = self.mUi.frontLabel
        self.mRightLabel = self.mUi.rightLabel
        self.mBackLabel = self.mUi.backLabel
        self.mLeftLabel = self.mUi.leftLabel
        self.mFrontProgressBar = self.mUi.frontProgressBar
        self.mRightProgressBar = self.mUi.rightProgressBar
        self.mBackProgressBar = self.mUi.backProgressBar
        self.mLeftProgressBar = self.mUi.leftProgressBar
        self.mFlyerCtrlGroupBox = self.mUi.flyerCtrlGroupBox
        self.sUpdateAcceleratorQuat.connect(self.UpdateAcceleratorQuat)
        self.mSaveCheckBox = self.mUi.saveCheckBox
        self.mSaveLineEdit = self.mUi.saveLineEdit
        self.mSavePushButton = self.mUi.savePushButton
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
        self.mAcceleratorCheckBox.setChecked(True)
        self.mSaveCheckBox.setChecked(True)
        self.mCapturePushButton.clicked.connect(self.ChangeState)
        self.mCommandPushButton.clicked.connect(self.StartFlyer)
        self.mSavePushButton.clicked.connect(self.SetSaveFileDir)
        self.mStopPushButton.clicked.connect(self.StopFlyer)

        # 加入波形控件
        self.mWaveWidget = FCWaveWidget()
        self.mWaveGroupBox = self.mUi.waveGroupBox
        vbox = QVBoxLayout()
        vbox.addWidget(self.mWaveWidget)
        self.mWaveGroupBox.setLayout(vbox)

        # 上行帧 表驱动 字典 
        self.updateFuncDict = {
                FCFrameType.FrameDataTimeAcceleratorDmpQuat: self.UpdateTimeAcceleratorDmpQuat,
                FCFrameType.FramePrintText : self.UpdatePrintText,
                FCFrameType.FrameError : self.UpdateErrorFrame,
            }

        self.commDict = {
                #键   : (类,        通信参数)
                '网络': (FCUdp,     (self.mIpLabel.text(), int(self.mPortLineEdit.text()))),
                '串口': (FCSerial,  (self.mComNameComboBox.currentText(), self.mBuadLineEdit.text())),
                }

        """
        interval = int(self.mIntervalLineEdit.text())
        frame = FCReqTimeAcceleratorDmpQuatFrame(interval)
        print("采样数据请求帧" , end = ':')
        frame.Print()

        accelerator = int(self.mAcceleratorSpinBox.value())
        frame = FCStartFrame(accelerator)
        print("发送加速帧" , end = ':')
        frame.Print() 
        
        frame = FCStopFrame()
        buf = frame.GetBytes()
        print("发送停止帧" , end = ':')
        frame.Print() 
        """

    def closeEvent(self, event):
        # 关闭后台接收线程
        self.mCapturing = False
        self.StopCapture()

    def SetSaveFileDir(self):
        print("未实现")

    def ChangeState(self, checked):
        if self.mCapturing: # 停止采集
            # 关闭保存的文件
            dataFilePath = self.mSaveLineEdit.text()
            self.mDataFile.close()

            self.mCapturing = False
            self.mCapturePushButton.setText("开始采集")
            self.mCommGroupBox.setEnabled(True)
            self.mDataGroupBox.setEnabled(True)
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
            self.mCommGroupBox.setEnabled(False)
            self.mDataGroupBox.setEnabled(False)
            self.StartCapture() 

    def StartFlyer(self, checked):
        if self._IsConnectted():
            accelerator = int(self.mAcceleratorSpinBox.value())
            print(accelerator) 
            
            frame = FCStartFrame(accelerator)
            buf = frame.GetBytes()
            print("发送加速帧" , end = ':')
            frame.Print()
            
            self.mComm.Write(buf)

    def StopFlyer(self, checked):
        if self._IsConnectted():
            frame = FCStopFrame()
            buf = frame.GetBytes()
            print("发送停止帧" , end = ':')
            frame.Print()

            self.mComm.Write(buf)

    def _IsConnectted(self):
        if self.mCapturing and self.mComm:
            return True
        else:
            return False

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
        # step1: 构造通信链路
        commClass = self.commDict[self.mCommType][0]
        paras = self.commDict[self.mCommType][1]
        print(paras)
        self.mComm = commClass(*paras)

        # step2: 获取下位机握手(有阻塞,所以使用后台线)
        self.mRecvThread = threading.Thread(target=self._RecvFunc)
        self.mRecvThread.start()

    def SendReqCaptureDataCmd(self):
        # TODO:由界面定制
        interval = int(self.mIntervalLineEdit.text())
        #print(interval) 

        frame = FCReqTimeAcceleratorDmpQuatFrame(interval)
        buf = frame.GetBytes()
        print("采样数据请求帧" , end = ':')
        frame.Print()

        self.mComm.Write(buf)
        print("开始监控")

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

    def UpdateTimeAcceleratorDmpQuat(self, frame):
        time = frame.GetTime()
        dmpQuat = frame.GetGmpQuat()
        euler = dmpQuat.ToEuler()
        accelerator = frame.GetAccelrator()

        # 加入数据
        self.mWaveWidget.Append(time, euler, accelerator)

        timeText = "运行:%7.1fs" % (time / 1000.0)
        dmpQuatText = dmpQuat.ToString()
        thetaText = "俯仰角:%+04.2fd" % euler.Theta()
        phiText   = "横滚角:%+04.2fd" % euler.Phi()
        psiText   = "偏航角:%+04.2fd" % euler.Psi()

        # 保存到文件
        #self.mDataFile.write(b"\r\n")
        #self.mDataFile.write(b"new:\r\n")
        self.mDataFile.write(frame.GetBytes())
        #self.mDataFile.write(b"\r\n")

        self.sUpdateAcceleratorQuat.emit(timeText, dmpQuatText, thetaText, phiText, psiText,
                accelerator[0], accelerator[1], accelerator[2], accelerator[3], accelerator[4])

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
            self.SendReqCaptureDataCmd()

        self.sAppendConsole.emit(text)

    def AppendConsole(self, text):
        # 有额外的换行
        #self.mConsolePlainTextEdit.appendPlainText(text)
        self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
        self.mConsolePlainTextEdit.insertPlainText(text)
        self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)

    def UpdateAcceleratorQuat(self, timeText, dmpQuatText, thetaText, phiText, psiText,
            frontAccelerator, rightAccelerator, backAccelerator, leftAccelerator, motherAccelerator):
        self.mRunTimeLabel.setText(timeText)
        self.mDmpQuatLabel.setText(dmpQuatText)
        self.mThetaLabel.setText(thetaText)
        self.mPhiLabel.setText(phiText)
        self.mPsiLabel.setText(psiText)

        #accelerator = (frontAccelerator, rightAccelerator, backAccelerator, leftAccelerator, motherAccelerator)
        #print(accelerator)

        # 更新油门
        # 设置最值
        self.mFrontProgressBar.setMaximum(motherAccelerator)
        self.mRightProgressBar.setMaximum(motherAccelerator)
        self.mBackProgressBar.setMaximum(motherAccelerator)
        self.mLeftProgressBar.setMaximum(motherAccelerator)

        self.mFrontProgressBar.setValue(frontAccelerator)
        self.mRightProgressBar.setValue(rightAccelerator)
        self.mBackProgressBar.setValue(backAccelerator)
        self.mLeftProgressBar.setValue(leftAccelerator)

        self.mFrontLabel.setText("% 3d" % int((frontAccelerator / motherAccelerator) * 100))
        self.mRightLabel.setText("% 3d" % int((rightAccelerator / motherAccelerator) * 100))
        self.mBackLabel.setText( "% 3d" % int((backAccelerator  / motherAccelerator) * 100))
        self.mLeftLabel.setText( "% 3d" % int((leftAccelerator  / motherAccelerator) * 100))

        # 使能控件
        self.mAcceleratorSpinBox.setMaximum(motherAccelerator)
        self.mAcceleratorLabel.setText("of % 4d" % motherAccelerator) 
        self.mAcceleratorSpinBox.setEnabled(True)
        self.mCommandPushButton.setEnabled(True)

    def UpdateErrorFrame(self, frame):
        return
        print("接收错误帧(%s:%s):" % (self.mComm.port, self.mComm.baudrate))
        frame.Print()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = FCCaptureWidget()
    win.show()
    sys.exit(app.exec_())


