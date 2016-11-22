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

# 协议帧
from frame.down import FCDownFrame, FCCtrlStartFrame, FCCtrlStopFrame

class FCOnlineWidget(QWidget): 
    sSendDownFrame = pyqtSignal(FCDownFrame, name = 'sSendDownFrame')

    def __init__(self, uiFile):
        super(FCOnlineWidget, self).__init__() 

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
        self.mPortLabel.setText("端口:" + str(gLocalPort))
        self.mDataPathLabel.setText("信息保存到:" + str(gSaveDataFileFullName))

        # 控制台文本输出
        self.mConsolePlainTextEdit = self.mUi.consolePlainTextEdit

        # 采样帧控制
        # 采样间隔
        self.mIntervalLineEdit = self.mUi.intervalLineEdit
        # 发送捕获信号按钮
        self.mSendDownFramePushButton = self.mUi.capturePushButton
        self.mSendDownFramePushButton.clicked.connect(self.Capture)

        # 油门控制
        self.mAcceleratorSpinBox = self.mUi.acceleratorSpinBox
        self.mStartPushButton = self.mUi.startPushButton
        self.mStopPushButton = self.mUi.stopPushButton
        self.mStartPushButton.clicked.connect(self.Start)
        self.mStopPushButton.clicked.connect(self.Stop)

        # 实时信息控件
        self.mRunTimeLabel = self.mUi.runTimeLabel
        self.mThetaLabel = self.mUi.thetaLabel
        self.mPhiLabel = self.mUi.phiLabel
        self.mPsiLabel = self.mUi.psiLabel

        # 为按键事件准备焦点策略
        self.setFocusPolicy(Qt.StrongFocus)

    def closeEvent(self, event):
        super(FCOnlineWidget, self).closeEvent(event)

    def Start(self):
        accelerator = int(self.mAcceleratorSpinBox.text()) 
        downFrame = FCCtrlStartFrame(accelerator)
        self.SendDownFrame(downFrame)

    def Stop(self):
        downFrame = FCCtrlStopFrame()
        self.SendDownFrame(downFrame)
        
    def SendDownFrame(self, frame): 
        self.sSendDownFrame.emit(frame)

    def RecvNewUpFrame(self, frame): 
        (tick, frameDict) = frame.ToFrameDict() 

        if frameDict['文本']:
            text = '[%05d]:%s' % (tick, frameDict['文本'])

            # 等效于 append 但是不加入换行
            self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
            self.mConsolePlainTextEdit.insertPlainText(text)
            self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
        else:
            label_str = '运行:% 6.1fs' %  (1.0 * tick / 1000)
            self.mRunTimeLabel.setText(label_str)

            euler = frameDict['欧拉角']
            label_str = '俯仰:%+6.1f' %  euler['俯仰角']
            self.mThetaLabel.setText(label_str)
            label_str = '横滚:%+6.1f' %  euler['横滚角']
            self.mPhiLabel.setText(label_str)
            label_str = '偏航:%+6.1f' %  euler['偏航角']
            self.mPsiLabel.setText(label_str)

