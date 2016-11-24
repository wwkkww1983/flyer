#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from PyQt5.uic import loadUiType, loadUi

from widget.online_widget import FCOnlineWidget
from frame.down import FCReqTimeAccelEuler

class FCFusionWidget(FCOnlineWidget):
    def __init__(self, uiFile):
        super(FCFusionWidget, self).__init__(uiFile) 
        # 基类FCOnlineWidget已经完成 读取/设置ui文件

        # 实时信息控件
        self.mDmpQuatLabel = self.mUi.dmpQuatLabel
        self.mAccelXLabel = self.mUi.accelXLabel
        self.mAccelYLabel = self.mUi.accelYLabel
        self.mAccelZLabel = self.mUi.accelZLabel

    def Capture(self):
        super(FCFusionWidget, self).Capture(FCReqTimeAccelEuler)

    def RecvNewUpFrame(self, frame): 
        super(FCFusionWidget, self).RecvNewUpFrame(frame) 
        (tick, frameDict) = frame.ToFrameDict() 

        # 非文本帧 更新加计数据
        if not frameDict['文本']: 
            accel = frameDict['加计']
            if accel:
                label_str = 'x:%+6.2f' %  accel['加计X']
                self.mAccelXLabel.setText(label_str)
                label_str = 'y:%+6.2f' %  accel['加计Y']
                self.mAccelYLabel.setText(label_str)
                label_str = 'z:%+6.2f' %  accel['加计Z']
                self.mAccelZLabel.setText(label_str)

