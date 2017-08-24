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

HOST, PORT = "54.242.251.112", 60000
BUFFER_SIZE = 2048

fileDir = os.path.dirname(os.path.realpath(__file__))
modelDir = os.path.dirname('/root/openface/models/')
dlibModelDir = os.path.join(modelDir, 'dlib')
openfaceModelDir = os.path.join(modelDir, 'openface')
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

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

def parse(args):  
    
    # Connection
    sock.connect((HOST, PORT))

    # train
    if args.mode == 'train':
        # data1 = '/root/openface/batch-represent/main.lua -outDir ' + args.trainDir + ' -data ' + args.outputDir
        # data2 = './server.py train ' + args.trainDir
        data = args.mode + ' ' + args.trainDir + ' ' + args.outputDir
    #classification
    else:
        # data = './server.py infer ' + args.classifierModel + ' ' + args.outputDir
        data = args.mode + ' ' + args.classifierModel + ' ' + args.outputDir
    
    sock.sendall(bytes(data + "\n"))
    received = str(sock.recv(4))

    sock.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    # $inputDir must includes a list of subfolders that 
    # each contains only images for single person
    # Doing so will allow you to do training work. 
    # You don't have to do this step if you just want to classify images.

    parser.add_argument('inputDir', type=str, help="Input image directory.")
    parser.add_argument('--dlibFacePredictor', type=str, help="Path to dlib's face predictor.",
                        default=os.path.join(dlibModelDir, "shape_predictor_68_face_landmarks.dat"))
    parser.add_argument(
        '--skipMulti', action='store_true', help="Skip images with more than one face.")
    subparsers = parser.add_subparsers(dest='mode', help="Mode")
    trainParser = subparsers.add_parser('train', help="Train a new classifier!")
    trainParser.add_argument('--trainDir', type=str, 
        help="The directory containing 'reps.csv' and 'labels.csv'", default = 'label')
    inferParser = subparsers.add_parser(
        'classify', help='Predict who an image contains from a trained classifier.')
    inferParser.add_argument(
        '--classifierModel', type=str, help='The Python pickle representing the classifier.', 
        default = '/root/openface/models/openface/celeb-classifier.nn4.small2.v1.pkl')
    parser.add_argument(
        '--outputDir', type=str, help="Output directory of aligned images.", default='alignedImg')
    
    args = parser.parse_args()

    start = time.time()
    # Alignmnet
    alignMain(args)
    print('alignment done!')
    # Transfer aligned images to cloud
    alignImg = 'scp -q -i /research/experiment/private_key/fog_Virginia.pem -r ' + args.outputDir + ' ec2-user@'+HOST+':/home/ec2-user/'
    print(alignImg)
    os.system(alignImg)
    print('transfer file done!')
    # Classification or training
    parse(args)
    print('execution done!')

    print('Execution took {} second'.format(time.time()-start))

