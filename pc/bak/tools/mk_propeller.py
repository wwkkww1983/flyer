#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
求解四螺旋桨的三角形坐标
"""

import sys
import math

if __name__ == '__main__': 
    angleStartList = [0, 30, 90, 120, 180, 210, 270, 300]
    pointList = []

    for angle in angleStartList:
        radian = math.radians(angle)
        point = (math.cos(radian), math.sin(radian))
        pointList.append(point)

    for p in pointList:
        print("%7.4f,%7.4f,%7.4f," % (p[0], p[1], 0.0))

