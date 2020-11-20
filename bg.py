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

ops = []
cam = wc.FakeWebcam('/dev/video20',1280,720)
conf = configparser.ConfigParser()
conf.read('options.txt')
ops.append(int(conf['ADJUSTMENTS']['topLeftX']))
ops.append(int(conf['ADJUSTMENTS']['topLeftY']))
ops.append(int(conf['ADJUSTMENTS']['width']))
ops.append(int(conf['ADJUSTMENTS']['height']))
ops.append(int(conf['ADJUSTMENTS']['showRect']))
ops.append(int(conf['ADJUSTMENTS']['blurFactor']))
del conf

cas = cv2.CascadeClassifier('lbpcascade_frontalface_improved.xml')
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH ,1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT,720)

while True:
	ret,frame = cap.read()
	if not ret:
		continue
	disp = frame.copy()
	frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	face = cas.detectMultiScale(frame,1.2,4)
	x = y = w = h = None
	for (x,y,w,h) in face:
		x-=ops[0]
		y-=ops[1]
		w+=ops[2]
		h+=ops[3]
		if ops[4] == 1:
			cv2.rectangle(disp, (x, y), (x+w, y+h), (255, 0, 0), 2)
	if any(t is None for t in (x,y,w,h)):
		continue
	sec = disp[int(y):int(y+h),int(x):int(x+w)].copy()
	disp = cv2.blur(disp,(ops[5],ops[5]))
	disp[int(y):int(y+h),int(x):int(x+w)] = sec
#	cv2.imshow('frame',disp)
	disp = cv2.cvtColor(disp,cv2.COLOR_BGR2RGB)
	cam.schedule_frame(disp)
	if cv2.waitKey(1) == ord('q'):
		break
cap.release()
cv2.destroyAllWindows()
