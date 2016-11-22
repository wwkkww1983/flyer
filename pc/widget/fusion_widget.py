#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from PyQt5.uic import loadUiType, loadUi

from widget.online_widget import FCOnlineWidget

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

    def Capture(self):
        print('FCFusionWidget.Capture')

