#!/usr/bin/env python2
# handler for passing arguments from client to server
# should be deployed on server

import socket
import os
from threading import Thread
import SocketServer
from SocketServer import ThreadingMixIn

TCP_IP = socket.gethostbyaddr("localhost")[0]
TCP_PORT = 60000
BUFFER_SIZE = 2048

print 'TCP_IP=',TCP_IP
print 'TCP_PORT=',TCP_PORT

class MyTCPSocketHandler(SocketServer.BaseRequestHandler):

    def handle(self):
        self.data = self.request.recv(BUFFER_SIZE).strip()
        os.system(self.data)

        self.request.sendall('Done');
        

if __name__ == "__main__":
    
    HOST, PORT = TCP_IP, TCP_PORT

    server = SocketServer.TCPServer((HOST, PORT), MyTCPSocketHandler)

    # activate the server
    server.handle_request()

