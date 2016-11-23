#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import enum

class FCFrameType(enum.Enum):
    # 以下类型基本帧类型(用户不使用)
    FrameUp             = 0x80000000
    FrameCtrl           = 0x40000000
    FrameReq            = 0x20000000
    FramePidSet         = 0x10000000

    FramePid            = 0x00000100
    FrameEuler          = 0x00000080
    FrameAccelerator    = 0x00000040
    FramePress          = 0x00000020
    FrameCompass        = 0x00000010
    FrameGyro           = 0x00000008
    FrameAccel          = 0x00000004
    FrameDmpQuat        = 0x00000002
    FramePrint          = 0x00000001
    FrameError          = 0xffffffff

    # 以下类型用户使用
    # dmp四元数采集请求帧
    FrameReqTimeAcceleratorDmpQuat  = FrameReq | FrameAccelerator | FrameDmpQuat
    FrameReqTimeAcceleratorEulerPid = FrameReq | FramePid | FrameEuler | FrameAccelerator
    FrameReqTimeAccelGyro           = FrameReq | FrameGyro | FrameAccel
    FrameReqTimeDmpQuatAccel        = FrameReq | FrameDmpQuat | FrameAccel

    # dmp四元数采集数据帧
    # 文本输出帧
    FrameDataAll = FramePid | FrameEuler | FrameAccelerator | FramePress | FrameCompass | FrameGyro | FrameAccel | FrameDmpQuat 
    FrameUpPrint = FrameUp | FramePrint

