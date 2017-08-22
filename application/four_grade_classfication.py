import serial
import time
import datetime
import sys
import paddle.v2 as paddle
from PIL import Image
import numpy as np
import cv2
from vgg import vgg_bn_drop


def multilayer_perceptron(img):
	hidden1 = paddle.layer.fc(input=img, size=128, act=paddle.activation.Relu())
	hidden2 = paddle.layer.fc(input=hidden1, size=64, act=paddle.activation.Relu())
	predict = paddle.layer.fc(input=hidden2, size=4, act=paddle.activation.Softmax())
	return predict

camera_port = 0
ramp_frames = 1
camera = cv2.VideoCapture(camera_port)

camera.set(3,320)
camera.set(4,240)

def get_image():
    retval, im = camera.read()
    return im
 
ser = serial.Serial('/dev/cu.usbserial', 9600, timeout=0.001)
words = '1'


for i in range(ramp_frames):
    print(datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
    temp = get_image()

datadim = 3 * 320 * 240
classdim = 4

# PaddlePaddle init
paddle.init(use_gpu=False, trainer_count=1)

image = paddle.layer.data(
    name="image", type=paddle.data_type.dense_vector(datadim))

net = multilayer_perceptron(image)
       
out = paddle.layer.fc(
    input=net, size=classdim, act=paddle.activation.Softmax())

lbl = paddle.layer.data(
    name="label", type=paddle.data_type.integer_value(classdim))
cost = paddle.layer.classification_cost(input=out, label=lbl)

while(1):
    line = ser.readline().decode()
    #print line
    #print(line)
    if line == '6':
        print("Receive "+line)
            #       for i in range(ramp_frames):
            #print(datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
            #temp = get_image()
        #print(datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
        #print("Taking image...")
        camera_capture = get_image()
        #print(datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))

            #print(datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
        #cv2.imwrite(file, camera_capture)


        im = np.array(camera_capture).astype(np.float32)

        im = im.transpose((2, 0, 1))  # CHW

        im = im.flatten()
        im = im / 255.0


        test_data = []
        test_data.append((im,))

        with open('params_pass_199.tar', 'r') as f:
            parameters = paddle.parameters.Parameters.from_tar(f)

        probs = paddle.infer(
            output_layer=out, parameters=parameters, input=test_data)
        lab = np.argsort(-probs)  # probs and lab are the results of one batch data
        #print "Label of image/dog.png is: %d" % lab[0][0]



        ser.write(str(lab[0][0]))
        print(str(lab[0][0]))
        #if words=="1":
        #    words="2"
        #elif words=="2":
        #    words="3"
        #elif words=="3":
        #    words="4"
        #elif words=="4":
        #    words="5"
        #elif words=="5":
        #    words="1"

ser.close()

 
del(camera)
