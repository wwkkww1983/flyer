#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import time
import struct
import socket
import threading

gBufSize = 1024

"""
接口有两个函数
1. 构造
2, Read
"""
class FCUdp():
    def __init__(self, ip, port):
        super(FCUdp, self).__init__()
        self.mIp = ip
        self.mPort = port
        self.mTargetAddrList = []
        self.mUdpSerSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # 监听套接字
        self.mUdpSerSock.bind((ip, port))
        self.mUdpSerSock.settimeout(0.1) # 100ms 超时值

        self.mUdpSendSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # 发送套接字(广播)
        self.mUdpSendSock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

        # 启动后台线程 读数据
        self.mRecvThread = threading.Thread(target=self._RecvFunc)
        self.mRecvThread.daemon = True # 主线程结束 子线程也结束
        self.mRunning = False
        self.mRecvBuf = b''

        self._StartRecvThread()

    def Close(self): 
        self._StopRecvThread()
        # FIXME: 关闭会出错
        #self.mUdpSerSock.close()
        #self.mUdpSendSock.close()

    def _StartRecvThread(self):
        self.mRunning = True
        self.mRecvThread.start() 

    def _StopRecvThread(self):
        self.mRunning = False
        self.mRecvThread.join(0.2) # 等待200ms 需要与recv超时值配合

    def _RecvFunc(self): 
        print("udp接收线程启动.")
        while self.mRunning: 
            try:
                (recvData, clientAddr) = self.mUdpSerSock.recvfrom(gBufSize)
            except Exception as e:  
                if isinstance(e, socket.timeout):  
                    #  超时
                    continue
            else:
                print(clientAddr, end = ":")
                # 更新 ip
                self.mTargetAddrList.append(clientAddr)
                print(recvData.decode('utf8'), end = ':')
                for c in recvData:
                    print("\\x%02x" % c, end = "")
                print()
                self.mRecvBuf = self.mRecvBuf + recvData
        print("udp接收线程结束.")

    def Read(self, length):
        rstBuf = b''
        recvBufLen = len(self.mRecvBuf)
        
        if length <= recvBufLen: # 数据够
            rstBuf = self.mRecvBuf[:length]
            self.mRecvBuf = self.mRecvBuf[length:]
        else: # 数据不够
            time.sleep(0.001 * length) # 一个byte等1ms 
            if length <= recvBufLen: # 数据够 
                rstBuf = self.mRecvBuf[:length]
                self.mRecvBuf = self.mRecvBuf[length:]
            else:
                return rstBuf

        return rstBuf

    def Write(self, buf):
        # FIXME:使用广播可能会成环
        for addr in self.mTargetAddrList: 
            print(addr)
            self.mUdpSendSock.sendto(buf, addr) 

if __name__ == '__main__': 
    localIP = socket.gethostbyname(socket.gethostname()) # 获取本地IP
    print("%s:%d" % (localIP, 8080))
    udp = FCUdp(localIP, 8080)

    time.sleep(1)

    while True: 
        data = udp.Read(10)
        if 0 != len(data):
            print(data)
            udp.Write(data)

    udp.Close()

