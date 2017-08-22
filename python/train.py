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


def reader_creator(flag):
	def reader():
		cnt = 0
		if flag == 'train':
			path = './train'
		else:
			path = './test'
		for label_dir in os.listdir(path):
			if('0' in label_dir or '1' in label_dir or '2' in label_dir or '3' in label_dir):
				label = label_dir[-1:]
				for dir in os.listdir(path+'/'+label_dir):
					if('.' not in dir):
						for image_name in os.listdir(path+'/'+label_dir+'/'+dir):
							if('png' in image_name):
								im = Image.open(path+'/'+label_dir+'/'+dir+'/'+image_name)
								#if path == './test':
								#	print path+'/'+label_dir+'/'+dir+'/'+image_name
								#	print label
								try:
									pass
									#im = im.resize((800, 600), Image.ANTIALIAS)
									#im = im.resize((32, 32), Image.ANTIALIAS)
								except:
									print 'lose frame'
									continue
								im = np.array(im).astype(np.float32)
								im = im.transpose((2, 0, 1))  # CHW
								im = im.flatten()
								im = im / 255.0
								if im.shape[0] != 230400:
									continue
								cnt = cnt+1
								yield im, int(label)
		print cnt
	return reader


	
def train():
	return reader_creator('train');

def test():
	return reader_creator('test');

def multilayer_perceptron(img):
#	hidden0 = paddle.layer.fc(input=img, size=1024, act=paddle.activation.Relu())
	hidden1 = paddle.layer.fc(input=img, size=128, act=paddle.activation.Relu())
	hidden2 = paddle.layer.fc(input=hidden1, size=64, act=paddle.activation.Relu())
	predict = paddle.layer.fc(input=hidden2, size=4, act=paddle.activation.Softmax())
	return predict

def main():
#    datadim = 3 * 32 * 32
    datadim = 3 * 320 *240
    classdim = 4

    # PaddlePaddle init
    paddle.init(use_gpu=False, trainer_count=1)

#    image = paddle.layer.data(
#        name="image", type=paddle.data_type.dense_vector(datadim))
    image = paddle.layer.data(
        name="image", height=320, width=240, type=paddle.data_type.dense_vector(datadim))

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
    parameters = paddle.parameters.create(cost)

    # Create optimizer
    momentum_optimizer = paddle.optimizer.Momentum(
        momentum=0.9,
        regularization=paddle.optimizer.L2Regularization(rate=0.0002 * 128),
        learning_rate=0.01 / 128.0,
        learning_rate_decay_a=0.1,
        learning_rate_decay_b=50000 * 100,
        learning_rate_schedule='discexp')

    # End batch and end pass event handler
    def event_handler(event):
        if isinstance(event, paddle.event.EndIteration):
            if event.batch_id % 5 == 0:
                print "\nPass %d, Batch %d, Cost %f, %s" % (
                    event.pass_id, event.batch_id, event.cost, event.metrics)
            else:
                sys.stdout.write('.')
                sys.stdout.flush()
        if isinstance(event, paddle.event.EndPass):
            # save parameters
            if event.pass_id > 190:
                with open('params_pass_%d.tar' % event.pass_id, 'w') as f:
                    parameters.to_tar(f)

            result = trainer.test(
                reader=paddle.batch(
					paddle.reader.shuffle(
                    test(), buf_size=50000), batch_size=128),
                feeding={'image': 0,
                         'label': 1})
            print "\nTest with Pass %d, %s" % (event.pass_id, result.metrics)

    # Create trainer
    trainer = paddle.trainer.SGD(
        cost=cost, parameters=parameters, update_equation=momentum_optimizer)
    trainer.train(
        reader=paddle.batch(
            paddle.reader.shuffle(
                train(), buf_size=20000),
            batch_size=128),
        num_passes=200,
        event_handler=event_handler,
        feeding={'image': 0,
                 'label': 1})




if __name__ == '__main__':
    main()
