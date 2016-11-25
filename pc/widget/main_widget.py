#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt5.QtWidgets import *

from PyQt5.QtCore import pyqtSignal
from PyQt5.uic import loadUiType, loadUi

from widget.frame_manager import FCFrameManager
from widget.analysis_widget import FCAnalysisWidget
from widget.fusion_widget import FCFusionWidget
from widget.pid_widget import FCPidWidget
# 控件配置
gWidgetConfigs = [
        (FCFusionWidget, r"widget/ui/fc_fusionWidget.ui", '姿态融合分析'),
        (FCPidWidget, r"widget/ui/fc_pidWidget.ui", 'PID分析'),
        (FCAnalysisWidget, r"widget/ui/fc_analysisWidget.ui", '离线分析')
        ]

class FCMainWidget(QTabWidget):
    def __init__(self):
        super(FCMainWidget, self).__init__() 

        # 帧管理器
        self.mFrameManager = FCFrameManager() 

        self.widget_list = []
        for widget_config in gWidgetConfigs:
            widget = widget_config[0](widget_config[1]) 
            self.widget_list.append(widget)
            self.addTab(widget, widget_config[2])

            # 绑定到实际发送 
            # 无sSendDownFrame表示不交互
            if hasattr(widget, 'sSendDownFrame'):
                widget.sSendDownFrame.connect(self.mFrameManager.SendDownFrame)

        # 默认数据采集有效
        self.setCurrentIndex(1)

        # 改变索引 需要 更换链接
        self.mFrameManager.sRecvNewUpFrame.connect(self.RecvNewUpFrame)

    def closeEvent(self, event):
        self.mFrameManager.close()
        for widget in self.widget_list:
            widget.close()

    def RecvNewUpFrame(self, frame):
        index = self.currentIndex()
        # 无RecvNewUpFrame表示不绘图
        if hasattr(self.widget_list[index], 'RecvNewUpFrame'):
            self.widget_list[index].RecvNewUpFrame(frame)

if __name__ == '__main__': 
    app = QApplication(sys.argv)
    win = FCWidget()
    win.show()
    sys.exit(app.exec_())

