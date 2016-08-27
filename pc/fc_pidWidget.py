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

from fc_waveWidget import FCWaveWidget

# 帧
from frame.frame_all import *
# 通信
from comm.comm_all import FCUdp

class FCPidWidget(QWidget): 
    sAppendConsole = pyqtSignal(str, name = 'sAppendConsole') 
    sUpdateAcceleratorEulerPid = pyqtSignal((int, tuple, tuple, tuple), name = 'sUpdateAcceleratorEulerPid')
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

        # 监控按钮
        self.mCapturePushButton = self.mUi.capturePushButton
        self.mCapturePushButton.clicked.connect(self.ChangeState)

        # 油门按钮
        self.mCommandPushButton = self.mUi.commandPushButton
        self.mCommandPushButton.clicked.connect(self.StartFlyer)
        # 停止按钮
        self.mStopPushButton = self.mUi.stopPushButton
        self.mStopPushButton.clicked.connect(self.StopFlyer)

        # 加入波形控件
        self.mWaveWidget = FCWaveWidget()
        self.mWaveGroupBox = self.mUi.waveGroupBox
        vbox = QVBoxLayout()
        vbox.addWidget(self.mWaveWidget)
        self.mWaveGroupBox.setLayout(vbox)

        # 本地保存文件相关控件
        self.mSaveLineEdit = self.mUi.saveLineEdit 

        # 时间控件
        self.mRunTimeLabel = self.mUi.runTimeLabel

        # 欧拉角控件
        self.mThetaLabel = self.mUi.thetaLabel
        self.mPhiLabel = self.mUi.phiLabel
        self.mPsiLabel = self.mUi.psiLabel

        # PID显示控件
        self.mThetaPidLabel = self.mUi.thetaPidLabel
        self.mPhiPidLabel = self.mUi.phiPidLabel
        self.mPsiPidLabel = self.mUi.psiPidLabel

        # PID控制控件
        self.mThetaPLineEdit = self.mUi.thetaPLineEdit
        self.mThetaILineEdit = self.mUi.thetaILineEdit
        self.mThetaDLineEdit = self.mUi.thetaDLineEdit
        self.mThetaPidPushButton = self.mUi.thetaPidPushButton
        self.mThetaPidPushButton.clicked.connect(self.SetThetaPid)
        self.mPhiPLineEdit = self.mUi.phiPLineEdit
        self.mPhiILineEdit = self.mUi.phiILineEdit
        self.mPhiDLineEdit = self.mUi.phiDLineEdit
        self.mPhiPidPushButton = self.mUi.phiPidPushButton
        self.mPhiPidPushButton.clicked.connect(self.SetPhiPid)
        self.mPsiPLineEdit = self.mUi.psiPLineEdit
        self.mPsiILineEdit = self.mUi.psiILineEdit
        self.mPsiDLineEdit = self.mUi.psiDLineEdit
        self.mPsiPidPushButton = self.mUi.psiPidPushButton
        self.mPsiPidPushButton.clicked.connect(self.SetPsiPid)
        """
        self.mThetaPLineEdit.setText('a')
        self.mThetaILineEdit.setText('b')
        self.mThetaDLineEdit.setText('c')
        self.mPhiPLineEdit.setText('d')
        self.mPhiILineEdit.setText('e')
        self.mPhiDLineEdit.setText('f')
        self.mPsiPLineEdit.setText('g')
        self.mPsiILineEdit.setText('h')
        self.mPsiDLineEdit.setText('i')
        """

        # 油门控件
        self.mFrontProgressBar = self.mUi.frontProgressBar
        self.mRightProgressBar = self.mUi.rightProgressBar
        self.mBackProgressBar = self.mUi.backProgressBar
        self.mLeftProgressBar = self.mUi.leftProgressBar
        self.mFrontLabel = self.mUi.frontLabel
        self.mRightLabel = self.mUi.rightLabel
        self.mBackLabel = self.mUi.backLabel
        self.mLeftLabel = self.mUi.leftLabel
        self.mAcceleratorSpinBox = self.mUi.acceleratorSpinBox
        self.mAcceleratorLabel = self.mUi.acceleratorLabel

        # 可打印帧显示控件
        self.mConsolePlainTextEdit = self.mUi.consolePlainTextEdit 
        self.sAppendConsole.connect(self.UpdatePrintTextWidget)

        # 采样帧信号
        self.sUpdateAcceleratorEulerPid.connect(self.UpdateTimeAcceleratorEulerPidWidget) 
        
        """
        frame = FCStopFrame()
        frame = FCStartFrame()
        frame = FCReqTimeAcceleratorDmpQuatFrame()
        frame = FCReqTimeAcceleratorEulerPid()
        buf = frame.GetBytes()
        print("帧" , end = ':')
        frame.Print()
        """

    def closeEvent(self, event):
        # 关闭后台接收线程
        self.mCapturing = False
        self.StopCapture()
        
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
        print(paras)

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
        #print(frameType)
        #frame.Print()

        # 表驱动
        updateFunc = self.updateFuncDict[frameType]
        updateFunc(frame)
        #self.update()

    def UpdateTimeAcceleratorEulerPid(self, frame):
        time = frame.GetTime()
        accelerator = frame.GetAccelrator()
        euler = frame.GetEuler()
        pid = frame.GetPid()

        """
        print('1')
        frame.Print()
        print(type(time))
        print(type(accelerator))
        print(type(euler))
        print(type(pid))
        print(time)
        print(accelerator)
        print(euler)
        print(pid)
        print('2')
        """
        # 加入数据
        self.mWaveWidget.Append(frame)

        # 保存到文件
        self.mDataFile.write(frame.GetBytes())
        self.sUpdateAcceleratorEulerPid.emit(time, accelerator, euler, pid) 
        
    def UpdateTimeAcceleratorEulerPidWidget(self, time, accelerator, euler, pid):
        timeText     = "运行:%8.1fs" % (time / 1000.0)
        self.mRunTimeLabel.setText(timeText)

        thetaText    = "俯仰:%+08.3f" % euler[0]
        phiText      = "横滚:%+08.3f" % euler[1]
        psiText      = "偏航:%+08.3f" % euler[2]
        self.mThetaLabel.setText(thetaText)
        self.mPhiLabel.setText(phiText)
        self.mPsiLabel.setText(psiText)

        thetaPidText = "俯仰:%+08.3f" % pid[0]
        phiPidText   = "横滚:%+08.3f" % pid[1]
        psiPidText   = "偏航:%+08.3f" % pid[2]
        self.mThetaPidLabel.setText(thetaPidText)
        self.mPhiPidLabel.setText(phiPidText)
        self.mPsiPidLabel.setText(psiPidText)

        frontAccelerator = accelerator[0]
        rightAccelerator = accelerator[1]
        backAccelerator  = accelerator[2]
        leftAccelerator  = accelerator[3]
        motherAccelerator= accelerator[4]

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
        self.mAcceleratorSpinBox.setMaximum(motherAccelerator)
        self.mAcceleratorLabel.setText("of % 4d" % motherAccelerator) 


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

    def UpdatePrintTextWidget(self, text):
        # 有额外的换行
        #self.mConsolePlainTextEdit.appendPlainText(text)
        self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
        self.mConsolePlainTextEdit.insertPlainText(text)
        self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)

    def UpdateErrorFrame(self, frame):
        print("接收错误帧(%s:%s):" % (self.mComm.port, self.mComm.baudrate))
        frame.Print()

    def SendReqCaptureDataCmd(self):
        # TODO:由界面定制
        interval = int(self.mIntervalLineEdit.text())
        #print(interval) 

        frame = FCReqTimeAcceleratorEulerPid(interval)
        buf = frame.GetBytes()
        print("采样数据请求帧" , end = ':')
        frame.Print()

        self.mComm.Write(buf)
        print("开始监控")

    def StartFlyer(self, checked):
        if self._IsConnectted():
            accelerator = int(self.mAcceleratorSpinBox.value())
            print(accelerator) 
            
            frame = FCCtrlStartFrame(accelerator)
            buf = frame.GetBytes()
            print("发送油门帧" , end = ':')
            frame.Print()
            
            self.mComm.Write(buf)
        else:
            print("未连接飞控板")

    def StopFlyer(self, checked):
        if self._IsConnectted():
            frame = FCCtrlStopFrame()
            buf = frame.GetBytes()
            print("发送停止帧" , end = ':')
            frame.Print()

            self.mComm.Write(buf)
        else:
            print("未连接飞控板")

    def _IsConnectted(self):
        if self.mCapturing and self.mComm:
            return True
        else:
            return False

    def SetThetaPid(self):
        self.SetPid('theta')

    def SetPhiPid(self):
        self.SetPid('phi')

    def SetPsiPid(self):
        self.SetPid('psi')

    def SetPid(self, euler_str):
        if self._IsConnectted():
            if 'theta' == euler_str: 
                print("俯仰pid帧" , end = ':')
            elif 'phi' == euler_str:
                print("横滚pid帧" , end = ':')
            elif 'psi' == euler_str:
                print("偏航pid帧" , end = ':') 
            else:
                print("%s不是有效欧拉角度." % euler_str) 
                return

            buf = frame.GetBytes()
            frame.Print()
            self.mComm.Write(buf)
        else:
            print("未连接飞控板")


if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = FCAnalysisWidget()
    win.show()
    sys.exit(app.exec_())

