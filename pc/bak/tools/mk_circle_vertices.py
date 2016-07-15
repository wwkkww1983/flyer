#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
求解圆的顶点坐标
"""

import sys
import math

if __name__ == '__main__': 
    if 3 != len(sys.argv):
        print("./mk_circle_vertiuce.py <x|y|z> <pointCounts>")
        exit(1)

    zeroAxis = str(sys.argv[1])
    pointCounts = int(sys.argv[2])

    if 'x' != zeroAxis and 'y' != zeroAxis and 'z' != zeroAxis:
        print("./mk_circle_vertiuce.py <x|y|z> <pointCounts>")
        exit(1)

    # 小于3 无法生成
    if pointCounts < 3:
        print("./mk_circle_vertiuce.py <x|y|z> <pointCounts>")
        print("pointCounts(%d) 太小不像圆." % pointCounts)
        exit(1)

    pointList = []

    i = 0
    step = 360.0 / pointCounts
    while i <= pointCounts:
        angle = i * step
        i += 1
        #print(angle)
        radian = math.radians(angle)
        #print(math.sin(radian))

        point = (math.cos(radian), math.sin(radian))
        pointList.append(point)


    for p in pointList:
        if 'x' == zeroAxis:
            print("%7.4f,%7.4f,%7.4f," % (0.0, p[0], p[1]))

        if 'y' == zeroAxis:
            print("%7.4f,%7.4f,%7.4f," % (p[0], 0.0, p[1]))

        if 'z' == zeroAxis:
            print("%7.4f,%7.4f,%7.4f," % (p[0], p[1], 0.0))

