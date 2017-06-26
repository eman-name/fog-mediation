#!/usr/bin/env python2

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import SocketServer

class Server(BaseHTTPRequestHandler):
  def do_POST(self):
    print(self.rfile.read(int(self.headers.getheader("Content-Length"))))
    self.send_response(200)
    self.send_header("Content-type", "text/plan")
    self.end_headers()
    self.wfile.write("OK")

def main(server_class=HTTPServer, handler_class=Server, port=9999):
  server_address = ('', port)
  httpd = server_class(server_address, handler_class)
  print("Ready! Waiting for connections...")
  httpd.serve_forever()

if __name__ == "__main__":
  main()
