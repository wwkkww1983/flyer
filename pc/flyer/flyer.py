#!/usr/bin/env python3

from socket import *

gFcIp = gethostbyname(gethostname()) # 获取本地IP
gFcPort = 8080
gFcAddr = (gFcIp, gFcPort)

class Flyer():
    def __init__(self):
        self.mUdpSocket = socket(AF_INET, SOCK_DGRAM)

    def __del__(self):
        self.mUdpSocket.close()

    def Send(self, data): 
        self.mUdpSocket.sendto(data, gFcAddr)
        pass

if __name__ == '__main__': 
    flyer = Flyer()
    flyer.Send(b'123')

