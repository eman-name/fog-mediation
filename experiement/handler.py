#!/usr/bin/env python2
# handler for passing arguments from client to server
# should be deployed on server

import socket
import os, errno
from threading import Thread
import SocketServer
from SocketServer import ThreadingMixIn

TCP_IP = socket.gethostbyaddr("172.17.0.2")[0]
TCP_PORT = 60000
BUFFER_SIZE = 2048

print 'TCP_IP=',TCP_IP
print 'TCP_PORT=',TCP_PORT

class MyTCPSocketHandler(SocketServer.BaseRequestHandler):

	# Handler for either training or classify
    def handle(self):
		self.data = self.request.recv(BUFFER_SIZE).strip()
		argList = self.data.split( )
		if(argList[0] == 'train'):
			data1 = '/root/openface/batch-represent/main.lua -outDir ' + argList[1] + ' -data ' + argList[2]
			data2 = './server.py train ' + argList[1]
			os.system(data1)
			os.system(data2)
		else:
			data3 = './server.py classify ' + argList[1] + ' ' + argList[2]
			os.system(data3)
		self.request.sendall('Done\n');
        

if __name__ == "__main__":
    
    HOST, PORT = TCP_IP, TCP_PORT
	# print('handler1')
    server = SocketServer.TCPServer((HOST, PORT), MyTCPSocketHandler)
    # print('handler2')
    # activate the server
    server.handle_request()

