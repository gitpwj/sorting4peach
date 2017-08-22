# Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserved
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License

import sys

import paddle.v2 as paddle

from PIL import Image

import numpy as np

from vgg import vgg_bn_drop

def multilayer_perceptron(img):
    #hidden0 = paddle.layer.fc(input=img, size=1024, act=paddle.activation.Relu())
    hidden1 = paddle.layer.fc(input=img, size=128, act=paddle.activation.Relu())
    hidden2 = paddle.layer.fc(input=hidden1, size=64, act=paddle.activation.Relu())
    predict = paddle.layer.fc(input=hidden2, size=4, act=paddle.activation.Softmax())
    return predict

def main():
    datadim = 3 * 320 * 240
    classdim = 4

    # PaddlePaddle init
    paddle.init(use_gpu=False, trainer_count=1)

    image = paddle.layer.data(
        name="image", type=paddle.data_type.dense_vector(datadim))

    # Add neural network config
    # option 1. resnet
    # net = resnet_cifar10(image, depth=32)
    # option 2. vgg
	#net = vgg_bn_drop(image)
    net = multilayer_perceptron(image)
	
    out = paddle.layer.fc(
        input=net, size=classdim, act=paddle.activation.Softmax())

    lbl = paddle.layer.data(
        name="label", type=paddle.data_type.integer_value(classdim))
    cost = paddle.layer.classification_cost(input=out, label=lbl)

    # Create parameters
    #parameters = paddle.parameters.Parameters.from_tar(gzip.open('params_pass_99.tar'))

    with open('params_pass_199.tar', 'r') as f:
        parameters = paddle.parameters.Parameters.from_tar(f)

    for i in range(0,10000):
        i = str(i)
        file = '../data/test/B1/'+i+'.png'
        try:
            im = Image.open(file)
        except:
            continue
        im = np.array(im).astype(np.float32)
        im = im.transpose((2, 0, 1))  # CHW
        im = im.flatten()
        im = im / 255.0
        if im.shape[0] != 230400:
            continue
            
        test_data = []
        test_data.append((im, ))


        probs = paddle.infer(
                             output_layer=out, parameters=parameters, input=test_data)
        lab = np.argsort(-probs)  # probs and lab are the results of one batch data
        print "Label of image/%s.png is: %d" % (i,lab[0][0])
        #print lab[0]
        #print probs[0]
        print probs[0][lab[0][0]]


if __name__ == '__main__':
    main()
