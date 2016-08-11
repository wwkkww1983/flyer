#!/usr/bin/env python3

from socket import *

#recvHost = '127.0.0.1'
recvHost = '192.168.1.2'
recvPort = 8080
recvAddr = (recvHost, recvPort)

sendData = b'123abc123abc'

udpSendSocket = socket(AF_INET, SOCK_DGRAM)

udpSendSocket.sendto(sendData, recvAddr)

udpSendSocket.close()

