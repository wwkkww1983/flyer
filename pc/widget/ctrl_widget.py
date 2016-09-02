#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from widget.frame_widget import FCFrameWidget

class FCCtrlWidget(FCFrameWidget): 
    def __init__(self, uiFile):
        super(FCCtrlWidget, self).__init__(uiFile) 
        self.mUi

    def closeEvent(self, event):
        super(FCCtrlWidget, self).closeEvent(event)

