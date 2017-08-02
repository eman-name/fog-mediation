#!/usr/bin/env python2
#
# Implemented and inspired from OpenFace project
# 
# Copyright 2015-2016 Carnegie Mellon University
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0

import argparse
import cv2
import os
import random
import shutil
import time
import socket, sys

import openface
import openface.helper
from openface.data import iterImgs

HOST, PORT = "52.23.169.184", 60000
BUFFER_SIZE = 2048

fileDir = os.path.dirname(os.path.realpath(__file__))
modelDir = os.path.dirname('/root/openface/models/')
dlibModelDir = os.path.join(modelDir, 'dlib')
openfaceModelDir = os.path.join(modelDir, 'openface')

def alignMain(args):
    openface.helper.mkdirP(args.outputDir)

    imgs = list(iterImgs(args.inputDir))

    # Shuffle so multiple versions can be run at once.
    random.shuffle(imgs)

    landmarkIndices = openface.AlignDlib.OUTER_EYES_AND_NOSE

    align = openface.AlignDlib(args.dlibFacePredictor)

    for imgObject in imgs:
        # print("=== {} ===".format(imgObject.path))
        outDir = os.path.join(args.outputDir, imgObject.cls)
        openface.helper.mkdirP(outDir)
        outputPrefix = os.path.join(outDir, imgObject.name)
        imgName = outputPrefix + ".png"

        if os.path.isfile(imgName):
            print("  + Already found, skipping.")
        else:
            rgb = imgObject.getRGB()
            if rgb is None:
                print("  + Unable to load.")
                outRgb = None
            else:
                outRgb = align.align(96, rgb, landmarkIndices=landmarkIndices, skipMulti=args.skipMulti)
                if outRgb is None:
                    print("  + Unable to align.")

            if outRgb is not None:
                outBgr = cv2.cvtColor(outRgb, cv2.COLOR_RGB2BGR)
                cv2.imwrite(imgName, outBgr)
            else:
                print("  + Unable to align.")

def parse():

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #classification
    data = './server.py infer /root/openface/models/openface/celeb-classifier.nn4.small2.v1.pkl ' + args.outputDir
    try:
    # connect to server 
    sock.connect((HOST, PORT))

    # send data
    sock.sendall(bytes(data + "\n"))

    received = str(sock.recv(BUFFER_SIZE))

    finally:
        # shut down
        sock.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('inputDir', type=str, help="Input image directory.")
    parser.add_argument('--dlibFacePredictor', type=str, help="Path to dlib's face predictor.",
                        default=os.path.join(dlibModelDir, "shape_predictor_68_face_landmarks.dat"))
    parser.add_argument(
        '--outputDir', type=str, help="Output directory of aligned images.", default=os.path.join(fileDir,'tmpAlignedImg'))
    parser.add_argument(
        '--skipMulti', action='store_true', help="Skip images with more than one face.")
    args = parser.parse_args()

    start = time.time()
    
    alignMain(args)
    os.system("scp -q -i /research/experiment/private_key/fog_Virginia.pem -r /research/experiment/img/ ec2-user@ec2-52-73-209-44.compute-1.amazonaws.com:/home/ec2-user/")
    parse()

    print('Execution took {} second'.format(time.time()-start))

