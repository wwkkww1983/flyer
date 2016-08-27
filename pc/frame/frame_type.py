#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from enum import Enum

class FCFrameType(Enum):
    # 以下类型基本帧类型(用户不使用)
    _Up                 = 0x80000000
    FrameFlyerCtrl      = 0x40000000
    FrameCaptureRequest = 0x20000000
    _Text               = 0x10000000

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
    FrameRequestTimeAcceleratorDmpQuat  = FrameCaptureRequest | _Accelerator | _DataDmpQuat | _DataTime
    FrameRequestTimeAcceleratorEulerPid = FrameCaptureRequest | _Pid | _Euler | _Accelerator | _DataTime

    # dmp四元数采集数据帧
    FrameDataTimeAcceleratorDmpQuat  = _Up | FrameCaptureRequest | _Accelerator | _DataDmpQuat | _DataTime
    FrameDataTimeAcceleratorEulerPid = _Up | FrameCaptureRequest | _Pid | _Euler | _Accelerator | _DataTime
    # 文本输出帧
    FramePrintText = _Up | _Text
    # 错误帧
    FrameError = 0xffffffff

if __name__ == '__main__':
    print("偷懒,先不写单元测试")

