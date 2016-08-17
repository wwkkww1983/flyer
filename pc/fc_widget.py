#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt5.QtWidgets import *

from PyQt5.QtCore import pyqtSignal
from PyQt5.uic import loadUiType, loadUi

from fc_captureWidget import FCCaptureWidget

class FCWidget(QTabWidget):
    def __init__(self):
        super(FCWidget, self).__init__() 

        # 飞行控制控件
        self.mFlyerWidget = QLabel("未实现")
        self.addTab(self.mFlyerWidget, "飞行控制")
        # 数据采集控件
        self.mCaptureWidget = FCCaptureWidget()
        self.addTab(self.mCaptureWidget, "数据采集")

        # 默认数据采集有效
        self.setCurrentIndex(1)

    def closeEvent(self, event):
        self.mCaptureWidget.close()

if __name__ == '__main__': 
    app = QApplication(sys.argv)
    win = FCWidget()
    win.show()
    sys.exit(app.exec_())

