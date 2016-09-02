#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from config import gLocalIP
from config import gLocalPort
from config import gSaveDataFileFullName

from widget.frame_widget import FCFrameWidget

class FCPidWidget(FCFrameWidget): 
    def __init__(self, uiFile):
        print(uiFile)
        super(FCPidWidget, self).__init__(uiFile) 

        #self.mUi

    def closeEvent(self, event):
        super(FCPidWidget, self).closeEvent(event)

