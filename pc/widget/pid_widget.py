#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from config import gLocalIP
from config import gLocalPort
from config import gSaveDataFileFullName
from config import gKeyStep

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.uic import loadUiType, loadUi 

from widget.wave_widget import FCWaveWidget
from widget.frame_widget import FCFrameWidget

# 协议帧
from frame.down import FCReqTimeAcceleratorEulerPid, FCCtrlStartFrame, FCCtrlStopFrame, FCPidSetFrame

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
        self.mSendDownFramePushButton.clicked.connect(self.Capture)

        # 油门控制
        self.mAcceleratorSpinBox = self.mUi.acceleratorSpinBox
        self.mStartPushButton = self.mUi.startPushButton
        self.mStopPushButton = self.mUi.stopPushButton
        self.mStartPushButton.clicked.connect(self.Start)
        self.mStopPushButton.clicked.connect(self.Stop)

        # pid参数控制
        self.mThetaPLineEdit = self.mUi.thetaPLineEdit
        self.mThetaILineEdit = self.mUi.thetaILineEdit
        self.mThetaDLineEdit = self.mUi.thetaDLineEdit
        self.mPhiPLineEdit = self.mUi.phiPLineEdit
        self.mPhiILineEdit = self.mUi.phiILineEdit
        self.mPhiDLineEdit = self.mUi.phiDLineEdit
        self.mPsiPLineEdit = self.mUi.psiPLineEdit
        self.mPsiILineEdit = self.mUi.psiILineEdit
        self.mPsiDLineEdit = self.mUi.psiDLineEdit

        self.mThetaPidPushButton = self.mUi.thetaPidPushButton
        self.mPhiPidPushButton = self.mUi.phiPidPushButton
        self.mPsiPidPushButton = self.mUi.psiPidPushButton
        self.mThetaPidPushButton.clicked.connect(self.PidThetaSet)
        self.mPhiPidPushButton.clicked.connect(self.PidPhiSet)
        self.mPsiPidPushButton.clicked.connect(self.PidPsiSet)

        # 实时信息控件
        self.mRunTimeLabel = self.mUi.runTimeLabel
        self.mThetaLabel = self.mUi.thetaLabel
        self.mPhiLabel = self.mUi.phiLabel
        self.mPsiLabel = self.mUi.psiLabel
        self.mThetaPidLabel = self.mUi.thetaPidLabel
        self.mPhiPidLabel = self.mUi.phiPidLabel
        self.mPsiPidLabel = self.mUi.psiPidLabel
        self.mFrontLabel = self.mUi.frontLabel
        self.mFrontProgressBar =  self.mUi.frontProgressBar
        self.mRightLabel = self.mUi.rightLabel
        self.mRightProgressBar =  self.mUi.rightProgressBar
        self.mBackLabel = self.mUi.backLabel
        self.mBackProgressBar =  self.mUi.backProgressBar
        self.mLeftLabel = self.mUi.leftLabel
        self.mLeftProgressBar =  self.mUi.leftProgressBar

        # 为按键事件准备焦点策略 (UI中已经设置)
        #self.setFocusPolicy(Qt.StrongFocus)

    def closeEvent(self, event):
        super(FCPidWidget, self).closeEvent(event)

    def Capture(self):
        if (True == self.mAcceletorCheckBox.isChecked()) and (True == self.mEulerCheckBox.isChecked()) and (True == self.mPidCheckBox.isChecked()):
            interval = int(self.mIntervalLineEdit.text())
            downFrame = FCReqTimeAcceleratorEulerPid(interval)
            self.SendDownFrame(downFrame)
            downFrame.Print()
        else:
            print("下行帧不符合要求.") 

    def Start(self):
        accelerator = int(self.mAcceleratorSpinBox.text()) 
        downFrame = FCCtrlStartFrame(accelerator)
        self.SendDownFrame(downFrame)
        downFrame.Print()

    def Stop(self):
        downFrame = FCCtrlStopFrame()
        self.SendDownFrame(downFrame)
        downFrame.Print()

    def PidThetaSet(self):
        euler_str = '俯仰PID'
        pidTuple = (float(self.mThetaPLineEdit.text()), float(self.mThetaILineEdit.text()), float(self.mThetaDLineEdit.text()),)
        downFrame = FCPidSetFrame(euler_str, pidTuple)
        self.SendDownFrame(downFrame)
        downFrame.Print()

    def PidPhiSet(self):
        euler_str = '横滚PID'
        pidTuple = (float(self.mPhiPLineEdit.text()), float(self.mPhiILineEdit.text()), float(self.mPhiDLineEdit.text()),)
        downFrame = FCPidSetFrame(euler_str, pidTuple)
        self.SendDownFrame(downFrame)
        downFrame.Print()

    def PidPsiSet(self):
        euler_str = '偏航PID'
        pidTuple = (float(self.mPsiPLineEdit.text()), float(self.mPsiILineEdit.text()), float(self.mPsiDLineEdit.text()),)
        downFrame = FCPidSetFrame(euler_str, pidTuple)
        self.SendDownFrame(downFrame)
        downFrame.Print()

    def RecvNewUpFrame(self, frame): 
        """
        接收新帧槽
        """
        #print("FCPidWidget.RecvNewUpFrame") 
        (tick, frameDict) = frame.ToFrameDict() 
        #print(tick)
        #frame.PrintDict()

        # 文本帧
        if frameDict['文本']:
            text = '[%05d]:%s' % (tick, frameDict['文本'])

            # 等效于 append 但是不加入换行
            self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
            self.mConsolePlainTextEdit.insertPlainText(text)
            self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
        else: 
            # 更新 时间/欧拉角/PID/油门 
            label_str = '运行:% 6.1fs' %  (1.0 * tick / 1000)
            self.mRunTimeLabel.setText(label_str)

            euler = frameDict['欧拉角']
            label_str = '俯仰:%+6.1f' %  euler['俯仰角']
            self.mThetaLabel.setText(label_str)
            label_str = '横滚:%+6.1f' %  euler['横滚角']
            self.mPhiLabel.setText(label_str)
            label_str = '偏航:%+6.1f' %  euler['偏航角']
            self.mPsiLabel.setText(label_str)

            pid = frameDict['PID']
            label_str = '俯仰:%+7.1f' %  pid['俯仰PID']
            self.mThetaPidLabel.setText(label_str)
            label_str = '横滚:%+7.1f' %  pid['横滚PID']
            self.mPhiPidLabel.setText(label_str)
            label_str = '偏航:%+7.1f' %  pid['偏航PID']
            self.mPsiPidLabel.setText(label_str)

            accelerator = frameDict['油门']
            value = accelerator['前'] * 1000
            label_str = '%3d' %  value
            self.mFrontLabel.setText(label_str)
            self.mFrontProgressBar.setValue(value)

            value = accelerator['右'] * 1000
            label_str = '%3d' %  value
            self.mRightLabel.setText(label_str)
            self.mRightProgressBar.setValue(value)

            value = accelerator['后'] * 1000
            label_str = '%3d' %  value
            self.mBackLabel.setText(label_str)
            self.mBackProgressBar.setValue(value)
            value = accelerator['左'] * 1000
            label_str = '%3d' %  value
            self.mLeftLabel.setText(label_str)
            self.mLeftProgressBar.setValue(value)

            # 将数据帧加入波形控件(波形控件自己会绘制)
            self.mWaveWidget.Append(tick, frameDict)
        #print()

    # 交互类函数
    def keyPressEvent(self, keyEvent):
        # 仅实现 俯仰PID设置
        # 1: + p
        # 2: - p
        # 3: + i
        # 4: - i
        # 5: + d
        # 6: - d
        key = keyEvent.key()
        if Qt.Key_1 == key:
            self._pidKeyChange(self.mThetaPLineEdit, '+')
            self.PidThetaSet()
        elif Qt.Key_2 == key:
            self._pidKeyChange(self.mThetaPLineEdit, '-')
            self.PidThetaSet()
        elif Qt.Key_3 == key:
            self._pidKeyChange(self.mThetaILineEdit, '+')
            self.PidThetaSet()
        elif Qt.Key_4 == key:
            self._pidKeyChange(self.mThetaILineEdit, '-')
            self.PidThetaSet()
        elif Qt.Key_5 == key:
            self._pidKeyChange(self.mThetaDLineEdit, '+')
            self.PidThetaSet()
        elif Qt.Key_6 == key:
            self._pidKeyChange(self.mThetaDLineEdit, '-')
            self.PidThetaSet()
        elif Qt.Key_Enter == key or Qt.Key_Return == key:
            self.Start()
        elif Qt.Key_Space == key:
            self.Stop()
        else:
            print("无效按键", key) 

    def keyReleaseEvent (self, keyEvent):
        pass

    def _pidKeyChange(self, lineEdit, add_or_dec): 
        val = 0
        if '+' == add_or_dec:
            val = float(lineEdit.text()) + gKeyStep
        elif '-' == add_or_dec:
            val = float(lineEdit.text()) - gKeyStep
        else:
            print("无效参数", key) 
            return
        val_str = "%.2f" % val
        lineEdit.setText(val_str)

