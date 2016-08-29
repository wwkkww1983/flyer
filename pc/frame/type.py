#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import enum

class FCFrameType(enum.Enum):
    # 以下类型基本帧类型(用户不使用)
    _Up                 = 0x80000000
    FrameCtrl           = 0x40000000
    FrameReq            = 0x20000000
    _Text               = 0x10000000
    FramePidSet         = 0x08000000

    _Pid                = 0x00000100
    _Euler              = 0x00000080
    _Accelerator        = 0x00000040
    _DataPress          = 0x00000020
    _DataCompass        = 0x00000010
    _DataGyro           = 0x00000008
    _DataAccel          = 0x00000004
    _DataDmpQuat        = 0x00000002
    _DataTime           = 0x00000001

    # 以下类型用户使用
    # dmp四元数采集请求帧
    FrameReqTimeAcceleratorDmpQuat  = FrameReq | _Accelerator | _DataDmpQuat | _DataTime
    FrameReqTimeAcceleratorEulerPid = FrameReq | _Pid | _Euler | _Accelerator | _DataTime

    # dmp四元数采集数据帧
    FrameDataTimeAcceleratorDmpQuat  = _Up | FrameReq | _Accelerator | _DataDmpQuat | _DataTime
    FrameDataTimeAcceleratorEulerPid = _Up | FrameReq | _Pid | _Euler | _Accelerator | _DataTime
    # 文本输出帧
    FramePrintText = _Up | _Text
    # 错误帧
    FrameError = 0xffffffff

