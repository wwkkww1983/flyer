#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from config import gLocalIP
from config import gLocalPort
from config import gSaveDataFileFullName

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.uic import loadUiType, loadUi 

from widget.wave_widget import FCWaveWidget
from widget.frame_widget import FCFrameWidget

# 协议帧
from frame.down import FCReqTimeAcceleratorEulerPid

class FCPidWidget(FCFrameWidget): 
    def __init__(self, uiFile):
        print(uiFile)
        super(FCPidWidget, self).__init__() 

        # 读取/设置ui文件
        UIClass = loadUiType(uiFile)
        self.mUi = UIClass[0]()
        self.mUi.setupUi(self) 

        # 加入波形控件
        self.mWaveWidget = FCWaveWidget()
        self.mWaveGroupBox = self.mUi.waveGroupBox
        vbox = QVBoxLayout()
        vbox.addWidget(self.mWaveWidget)
        self.mWaveGroupBox.setLayout(vbox)

        # 基本配置
        self.mIpLabel = self.mUi.ipLabel
        self.mPortLabel = self.mUi.portLabel
        self.mDataPathLabel = self.mUi.dataPathLabel
        self.mIpLabel.setText("IP:" + str(gLocalIP))
        self.mPortLabel.setText("Port:" + str(gLocalPort))
        self.mDataPathLabel.setText("信息保存到:" + str(gSaveDataFileFullName))

        # 控制台文本输出
        self.mConsolePlainTextEdit = self.mUi.consolePlainTextEdit

        # 采样帧控制
        # 采样间隔
        self.mIntervalLineEdit = self.mUi.intervalLineEdit
        # 下行帧复选
        self.mDmpQuatCheckBox = self.mUi.dmpQuatCheckBox
        self.mAccelCheckBox = self.mUi.accelCheckBox
        self.mGyroCheckBox = self.mUi.gyroCheckBox
        self.mCompassCheckBox = self.mUi.compassCheckBox
        self.mPressCheckBox = self.mUi.pressCheckBox
        self.mAcceletorCheckBox = self.mUi.acceletorCheckBox
        self.mEulerCheckBox = self.mUi.eulerCheckBox
        self.mPidCheckBox = self.mUi.pidCheckBox
        # 发送捕获信号按钮
        self.mSendDownFramePushButton = self.mUi.capturePushButton
        self.mSendDownFramePushButton.clicked.connect(self.SendFrame)

    def closeEvent(self, event):
        super(FCPidWidget, self).closeEvent(event)

    def SendFrame(self):
        if (True == self.mAcceletorCheckBox.isChecked()) and (True == self.mEulerCheckBox.isChecked()) and (True == self.mPidCheckBox.isChecked()):
            interval = int(self.mIntervalLineEdit.text())
            downFrame = FCReqTimeAcceleratorEulerPid(interval)
            self.SendDownFrame(downFrame)
            #print("FCReqTimeAcceleratorEulerPid已经发送.") 
        else:
            print("下行帧不符合要求.") 

    def RecvNewUpFrame(self, frame): 
        """
        接收新帧槽
        """
        #print("FCPidWidget.RecvNewUpFrame") 
        (time, frameDict) = frame.ToFrameDict() 
        #print(time)
        #frame.PrintDict()

        # 文本帧
        if frameDict['文本']:
            text = '[%05d]:%s' % (time, frameDict['文本'])

            # 等效于 append 但是不加入换行
            self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
            self.mConsolePlainTextEdit.insertPlainText(text)
            self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
        else: 
            self.mWaveWidget.Append(time, frameDict)

        #print()

