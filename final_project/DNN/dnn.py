#!/usr/bin/env python
from __future__ import division, print_function
from __future__ import absolute_import

# imports
import tensorflow as tf
import cv2
import sys
import os
import time
import math
import numpy as np
import serial

# initialize serial connection
try:
    ser = serial.Serial("/dev/ttyAMA1", 115200)
    print("serial connection established on /dev/ttyAMA1")
except serial.SerialException as e:
    print("error initializing serial connection: {}".format(e))
    sys.exit(1)

# radian <-> degree conversion functions
def deg2rad(deg):
    return deg * math.pi / 180.0

def rad2deg(rad):
    return 180.0 * rad / math.pi

# load the model
sess = tf.InteractiveSession()
saver = tf.train.Saver()
model_load_path = "model/model.ckpt"
saver.restore(sess, model_load_path)
print("model loaded successfully.")

# video processing
vid_path = 'epoch-1.avi'
assert os.path.isfile(vid_path), "video file {} not found.".format(vid_path)
cap = cv2.VideoCapture(vid_path)
print("processing video {}.".format(vid_path))

curFrame = 0
NFRAMES = 1000
count = 0
time_start = time.time()  # Initialize the start time for FPS calculation
tot_time_list = []  # Initialize the total time list for statistics

while curFrame < NFRAMES:
    ret, img = cap.read()
    if not ret:
        print("end of video or error reading frame.")
        break

    img = cv2.resize(img, (200, 66)) / 255.0

    rad = model.y.eval(feed_dict={model.x: [img]})[0][0]
    deg = str(int(rad2deg(rad)))

    # send every 4th prediction to the HiFive
    if count % 4 == 0:
        try:
            ser.write((deg + '\n').encode('utf-8'))  # Explicitly encode string for Python 2
            print("sent {} through serial communication.".format(deg))
        except Exception as e:
            print("error sending data: {}".format(e))

    curFrame += 1
    count += 1

cap.release()
print("video processing complete.")

# Calculate and output FPS/frequency
fps = curFrame / (time.time() - time_start)
print('completed inference, total frames: {}, average fps: {} Hz'.format(curFrame + 1, round(fps, 1)))

# Calculate and display statistics of the total inferencing times
if tot_time_list:
    print("count: {}".format(len(tot_time_list)))
    print("mean: {}".format(np.mean(tot_time_list)))
    print("max: {}".format(np.max(tot_time_list)))
    print("99.999pct: {}".format(np.percentile(tot_time_list, 99.999)))
    print("99.99pct: {}".format(np.percentile(tot_time_list, 99.99)))
    print("99.9pct: {}".format(np.percentile(tot_time_list, 99.9)))
    print("99pct: {}".format(np.percentile(tot_time_list, 99)))
    print("min: {}".format(np.min(tot_time_list)))
    print("median: {}".format(np.median(tot_time_list)))
    print("stdev: {}".format(np.std(tot_time_list)))
else:
    print("No timing data collected.")
