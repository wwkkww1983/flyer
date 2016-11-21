#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt5.QtWidgets import *

from PyQt5.QtCore import pyqtSignal
from PyQt5.uic import loadUiType, loadUi

from widget.analysis_widget import FCAnalysisWidget
from widget.fusion_widget import FCFusionWidget
from widget.pid_widget import FCPidWidget

class FCMainWidget(QTabWidget):
    def __init__(self):
        super(FCMainWidget, self).__init__() 

        # 姿态融合分析控件
        self.mFusionWidget = FCFusionWidget(r"widget/ui/fc_fusionWidget.ui")
        self.addTab(self.mFusionWidget, "姿态融合分析")

        # pid分析控件
        self.mPidWidget = FCPidWidget(r"widget/ui/fc_pidWidget.ui")
        self.addTab(self.mPidWidget, "PID分析")

        # 离线分析控件
        self.mAnalysisWidget = FCAnalysisWidget(r"widget/ui/fc_analysisWidget.ui")
        self.addTab(self.mAnalysisWidget, "离线分析")

        # 默认数据采集有效
        self.setCurrentIndex(0)

    def closeEvent(self, event):
        self.mPidWidget.close()
        self.mAnalysisWidget.close()
        self.mFusionWidget.close()

if __name__ == '__main__': 
    app = QApplication(sys.argv)
    win = FCWidget()
    win.show()
    sys.exit(app.exec_())

