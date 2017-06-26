#!/usr/bin/env python

import cv2
import numpy
import threading
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from SocketServer import ThreadingMixIn
import time
capture=None

class CamHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path.endswith('.mjpg'):
            self.send_response(200)
            self.send_header('Content-type','multipart/x-mixed-replace; boundary=--jpgboundary')
            self.end_headers()
            previous = 0
            while True:
                try:
                    rc,img = capture.read()
                    if not rc:
                        continue
                    current = time.time()
                    if current - previous < 1.0:
                        continue
                    previous = current
                    _, jpg = cv2.imencode(".jpg", img, [int(cv2.IMWRITE_JPEG_QUALITY), 75])
                    img = numpy.array(jpg).tostring()
                    self.wfile.write("--jpgboundary\r\n")
                    self.send_header('Content-Type','image/jpeg')
                    self.send_header('Content-Length',str(len(img)))
                    self.end_headers()
                    self.wfile.write(img)
                    self.wfile.write("\r\n")
                except KeyboardInterrupt:
                    break
            return
        if self.path.endswith('.html'):
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            self.wfile.write('<html><head></head><body>')
            self.wfile.write('<img src="http://127.0.0.1:9999/cam.mjpg"/>')
            self.wfile.write('</body></html>')
            return


class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    """Handle requests in a separate thread."""

def main():
    global capture
    capture = cv2.VideoCapture(0)
    try:
        capture.set(cv2.cv.CV_CAP_PROP_FPS, 1)
        capture.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH, 1280)
        capture.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT, 720)
    except:
        capture.set(cv2.CAP_PROP_FPS, 1)
        capture.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
        capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
    try:
        server = ThreadedHTTPServer(('0.0.0.0', 9999), CamHandler)
        print "server started"
        server.serve_forever()
    except KeyboardInterrupt:
        capture.release()
        server.socket.close()

if __name__ == '__main__':
    main()
