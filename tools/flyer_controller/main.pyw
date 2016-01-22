#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
os:winxp sp3 32
pyqt:PyQt4-4.11.4-gpl-Py3.4-Qt4.8.7-x32
opengl:pip install PyOpenGL(PyOpenGL3.1.0)
"""

import sys
from PyQt4 import QtGui

from fc_widgets import FCWindow

app = QtGui.QApplication(sys.argv)
win= FCWindow()
win.move(0,0)
win.show()
sys.exit(app.exec_())


