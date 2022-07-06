import cv2
import numpy as np
from elements.yolo import OBJ_DETECTION
import timeit
import statistics
import socket
import pickle
import struct
Object_classes = ['person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus', 'train', 'truck', 'boat', 'traffic light',
                'fire hydrant', 'stop sign', 'parking meter', 'bench', 'bird', 'cat', 'dog', 'horse', 'sheep', 'cow',
                'elephant', 'bear', 'zebra', 'giraffe', 'backpack', 'umbrella', 'handbag', 'tie', 'suitcase', 'frisbee',
                'skis', 'snowboard', 'sports ball', 'kite', 'baseball bat', 'baseball glove', 'skateboard', 'surfboard',
                'tennis racket', 'bottle', 'wine glass', 'cup', 'fork', 'knife', 'spoon', 'bowl', 'banana', 'apple',
                'sandwich', 'orange', 'broccoli', 'carrot', 'hot dog', 'pizza', 'donut', 'cake', 'chair', 'couch',
                'potted plant', 'bed', 'dining table', 'toilet', 'tv', 'laptop', 'mouse', 'remote', 'keyboard', 'cell phone',
                'microwave', 'oven', 'toaster', 'sink', 'refrigerator', 'book', 'clock', 'vase', 'scissors', 'teddy bear',
                'hair drier', 'toothbrush' ]

Object_detector = OBJ_DETECTION('weights/yolov5s.pt', Object_classes)
print("here")

framecounter = 0
times = []
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('192.168.80.210', 1337))
# To flip the image, modify the flip_method parameter (0 and 2 are the most common)

cap = cv2.VideoCapture('cut_video.mp4')
out = cv2.VideoWriter('outpy.avi',cv2.VideoWriter_fourcc('X','V','I','D'), 30, (1280,720))
while cap.isOpened():
    ret, frame = cap.read()
    if ret:
        # detection process
        time_begin = timeit.default_timer()
        objs = Object_detector.detect(frame)
        time_end = timeit.default_timer()
        times.append(time_end - time_begin)

        # plotting
        for obj in objs:
            # print(obj)
            label = obj['label']
            score = obj['score']
            print(label)
            print(score)
            [(xmin,ymin),(xmax,ymax)] = obj['bbox']
            frame = cv2.rectangle(frame, (xmin,ymin), (xmax,ymax), (0,0,0), 2) 
            frame = cv2.putText(frame, f'{label} ({str(score)})', (xmin,ymin), cv2.FONT_HERSHEY_SIMPLEX , 0.75, (0,0,0), 1, cv2.LINE_AA)
        framecounter += 1
        #out.write(frame)
        data = pickle.dumps(frame)
        message_size=struct.pack("L",len(data))
        s.sendall(message_size+data)
    if framecounter >= 300:
        cap.release()
        out.release()
        # calculate median of times
        print(statistics.median(times))
        print("FPS: {}".format(1 / statistics.median(times)))
        break


