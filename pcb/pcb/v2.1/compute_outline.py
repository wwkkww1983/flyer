#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import math

# 马达孔外径
d1 = 10.0
r1 = d1 / 2
# 螺旋桨直径
d2 = 55.0
r2 = d2 / 2
# 一个角度常量
theta = math.radians(22.5)

def func(rect_length): 
    x = rect_length / 2
    if(x < r1):
        print("布线正方形太小")
        exit();

    sintheta = math.sin(theta)
    sqrt2 = math.sqrt(2)

    x1 = x
    x2 = (x - r1)*sintheta/(1 - sintheta)
    x3 = r2/sqrt2 + (1-0.5*sqrt2)*r1

    #print(sintheta)
    #print(sqrt2)
    #print(x1)
    #print(x2)
    #print(x3)

    return 2*(x1+x2+x3)

if __name__ == '__main__':
    for length in range(20, 101):
        print("%.2f,%.2f" % (length, func(length)))



