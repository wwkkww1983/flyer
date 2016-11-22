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

from widget.online_widget import FCOnlineWidget

# 协议帧
from frame.down import FCReqTimeAcceleratorEulerPid, FCCtrlStartFrame, FCCtrlStopFrame, FCPidSetFrame

class FCPidWidget(FCOnlineWidget):
    def __init__(self, uiFile):
        super(FCPidWidget, self).__init__(uiFile)
        # 基类FCOnlineWidget已经完成 读取/设置ui文件

        # 下行帧复选
        self.mAcceletorCheckBox = self.mUi.acceletorCheckBox
        self.mEulerCheckBox = self.mUi.eulerCheckBox
        self.mPidCheckBox = self.mUi.pidCheckBox

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

    def Capture(self):
        if (True == self.mAcceletorCheckBox.isChecked()) and (True == self.mEulerCheckBox.isChecked()) and (True == self.mPidCheckBox.isChecked()):
            interval = int(self.mIntervalLineEdit.text())
            downFrame = FCReqTimeAcceleratorEulerPid(interval)
            self.SendDownFrame(downFrame)
        else:
            print("下行帧不符合要求.") 

    def PidThetaSet(self):
        euler_str = '俯仰PID'
        pidTuple = (float(self.mThetaPLineEdit.text()), float(self.mThetaILineEdit.text()), float(self.mThetaDLineEdit.text()),)
        downFrame = FCPidSetFrame(euler_str, pidTuple)
        self.SendDownFrame(downFrame)

    def PidPhiSet(self):
        euler_str = '横滚PID'
        pidTuple = (float(self.mPhiPLineEdit.text()), float(self.mPhiILineEdit.text()), float(self.mPhiDLineEdit.text()),)
        downFrame = FCPidSetFrame(euler_str, pidTuple)
        self.SendDownFrame(downFrame)

    def PidPsiSet(self):
        euler_str = '偏航PID'
        pidTuple = (float(self.mPsiPLineEdit.text()), float(self.mPsiILineEdit.text()), float(self.mPsiDLineEdit.text()),)
        downFrame = FCPidSetFrame(euler_str, pidTuple)
        self.SendDownFrame(downFrame)

    def RecvNewUpFrame(self, frame): 
        (tick, frameDict) = frame.ToFrameDict() 

        # 非文本帧 更新时间/欧拉角/PID/油门
        if not frameDict['文本']:
            pid = frameDict['PID']
            label_str = '俯仰:%+7.1f' %  pid['俯仰PID']
            self.mThetaPidLabel.setText(label_str)
            label_str = '横滚:%+7.1f' %  pid['横滚PID']
            self.mPhiPidLabel.setText(label_str)
            label_str = '偏航:%+7.1f' %  pid['偏航PID']
            self.mPsiPidLabel.setText(label_str)

            # 归一化时 -0.5 还原
            accelerator = frameDict['油门']
            value = (0.5 + accelerator['前']) * 1000
            label_str = '%3d' %  value
            self.mFrontLabel.setText(label_str)
            self.mFrontProgressBar.setValue(value)
            value = (0.5 + accelerator['右']) * 1000
            label_str = '%3d' %  value
            self.mRightLabel.setText(label_str)
            self.mRightProgressBar.setValue(value)
            value = (0.5 + accelerator['后']) * 1000
            label_str = '%3d' %  value
            self.mBackLabel.setText(label_str)
            self.mBackProgressBar.setValue(value)
            value = (0.5 + accelerator['左']) * 1000
            label_str = '%3d' %  value
            self.mLeftLabel.setText(label_str)
            self.mLeftProgressBar.setValue(value)

            # 将数据帧加入波形控件(波形控件自己会绘制)
            self.mWaveWidget.Append(tick, frameDict)

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

