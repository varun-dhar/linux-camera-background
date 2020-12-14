#!/usr/bin/python3
'''
MIT License

Copyright (c) 2020 Varun Dhar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''
import cv2
import numpy as np
import configparser
import pyfakewebcam as wc
import tensorflow as tf
from tf_bodypix.api import download_model, load_model, BodyPixModelPaths

model = load_model(download_model(BodyPixModelPaths.MOBILENET_FLOAT_50_STRIDE_16))

ops = []
bgimage = None
cam = wc.FakeWebcam('/dev/video20',1280,720)
conf = configparser.ConfigParser()
conf.read('options.txt')
ops.append(int(conf['ADJUSTMENTS']['blurFactor']))
ops.append(int(conf['ADJUSTMENTS']['useImageBg']))
bgimage = conf['ADJUSTMENTS']['backgroundFile']
ops.append(int(conf['ADJUSTMENTS']['maskSensitivity']))
del conf
ops[0] = ops[0]+1 if (ops[0]%2)==0 else ops[0]

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH ,1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT,720)

while True:
	ret,frame = cap.read()
	mask = model.predict_single(frame)
	mask = mask.get_mask(threshold=ops[2]/100)
	inv_mask = 1-mask
	clear = frame.copy()
	if ops[1] == 0:
		frame = cv2.blur(frame,(ops[0],ops[0]))
	else:
		frame = cv2.imread(bgimage)
		frame = cv2.resize(frame,(1280,720))
	for i in range(frame.shape[2]):
		clear[:,:,i] = clear[:,:,i]*mask[:,:,0] + frame[:,:,i]*inv_mask[:,:,0]
	clear = cv2.cvtColor(clear,cv2.COLOR_BGR2RGB)
	cam.schedule_frame(clear)
cap.release()
cv2.destroyAllWindows()
