#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from PyQt5.uic import loadUiType, loadUi

from widget.online_widget import FCOnlineWidget
from frame.down import FCReqTimeDmpQuatAccel

class FCFusionWidget(FCOnlineWidget):
    def __init__(self, uiFile):
        super(FCFusionWidget, self).__init__(uiFile) 
        # 基类FCOnlineWidget已经完成 读取/设置ui文件

        # 下行帧复选
        self.mDmpQuatCheckBox = self.mUi.dmpQuatCheckBox
        self.mAccelCheckBox = self.mUi.accelCheckBox
        self.mGyroCheckBox = self.mUi.gyroCheckBox
        self.mCompassCheckBox = self.mUi.compassCheckBox
        self.mPressCheckBox = self.mUi.pressCheckBox

        # 实时信息控件
        self.mDmpQuatLabel = self.mUi.dmpQuatLabel
        self.mAccelXLabel = self.mUi.accelXLabel
        self.mAccelYLabel = self.mUi.accelYLabel
        self.mAccelZLabel = self.mUi.accelZLabel

    def Capture(self):
        if True == self.mAccelCheckBox.isChecked() and (True == self.mDmpQuatCheckBox.isChecked()):
            interval = int(self.mIntervalLineEdit.text())
            downFrame = FCReqTimeDmpQuatAccel(interval)
            self.SendDownFrame(downFrame)
        else:
            print("FCFusionWidget.Capture下行帧不符合要求.")

    def RecvNewUpFrame(self, frame): 
        (tick, frameDict) = frame.ToFrameDict() 

        # 非文本帧 更新/DmpQuat/加计数据
        if not frameDict['文本']: 
            dmpQuat = frameDict['DMP四元数']
            dmpQuatStr = "%s" % dmpQuat
            self.mDmpQuatLabel.setText(dmpQuatStr)

            accel = frameDict['加计']
            label_str = 'x:%+6.2f' %  accel['加计X']
            self.mAccelXLabel.setText(label_str)
            label_str = 'y:%+6.2f' %  accel['加计Y']
            self.mAccelYLabel.setText(label_str)
            label_str = 'z:%+6.2f' %  accel['加计Z']
            self.mAccelZLabel.setText(label_str)

            # 将数据帧加入波形控件(波形控件自己会绘制)
            self.mWaveWidget.Append(tick, frameDict)

