import numpy as np
import json
import os
import os.path as osp
import sys
import google.protobuf as pb
from argparse import ArgumentParser


CAFFE_ROOT = '../../'
if osp.join(CAFFE_ROOT, 'python') not in sys.path:
    sys.path.insert(0, osp.join(CAFFE_ROOT, 'python'))
import caffe
from caffe.proto.caffe_pb2 import NetParameter, LayerParameter


def load_config(config_file):
    with open(config_file, 'r') as fp:
        conf = json.load(fp)
    return conf


def vh_decompose(conv, K):
    def _create_new(name):
        new_ = LayerParameter()
        new_.CopyFrom(conv)
        new_.name = name
        new_.convolution_param.ClearField('kernel_size')
        new_.convolution_param.ClearField('pad')
        new_.convolution_param.ClearField('stride')
        return new_
    conv_param = conv.convolution_param
    # vertical
    print("TEST: Conv Name: ", + conv.name)
    v = _create_new(conv.name + '_v')
    del(v.top[:])
    v.top.extend([v.name])
    print "work"
    print len(v.param)
    #v.param[1].lr_mult = 0
    if (len(v.param) > 1):
        v.param[1].lr_mult = 0
    
    v_param = v.convolution_param
    v_param.num_output = K
    v_param.kernel_h = conv_param.kernel_size[0]
    v_param.kernel_w = 1
    #v_param.kernel_h, v_param.kernel_w = conv_param.kernel_size, 1
    if len(conv_param.pad):
        v_param.pad_h = conv_param.pad[0]
    else:
        v_param.pad_h = 0
    v_param.pad_w = 0
    if len(conv_param.stride):
        v_param.stride_h = conv_param.stride[0]
    else:
        v_param.stride_h = 1
    v_param.stride_w = 1
    # horizontal
    h = _create_new(conv.name + '_h')
    del(h.bottom[:])
    h.bottom.extend(v.top)
    h_param = h.convolution_param
    h_param.kernel_h, h_param.kernel_w = 1, conv_param.kernel_size[0]
    #h_param.pad_h, h_param.pad_w = 0, conv_param.pad
    h_param.pad_h = 0
    if len(conv_param.pad):
        h_param.pad_w = conv_param.pad[0]
    else:
        h_param.pad_w = 0
    
    
    #h_param.stride_h, h_param.stride_w = 1, conv_param.stride
    h_param.stride_h =1
    if len(conv_param.stride ):
        h_param.stride_w = conv_param.stride[0]
    else:
        h_param.stride_w = 1
    return v, h


def make_lowrank_model(input_file, conf, output_file):
    with open(input_file, 'r') as fp:
        net = NetParameter()
        pb.text_format.Parse(fp.read(), net)
    print(conf.keys())
    new_layers = []
    for layer in net.layer:
        if not layer.name in conf.keys():
            new_layers.append(layer)
            continue
    	print("TEST: Conv Name: ", + layer.name)
	v, h = vh_decompose(layer, conf[layer.name])
        new_layers.extend([v, h])
    new_net = NetParameter()
    new_net.CopyFrom(net)
    del(new_net.layer[:])
    new_net.layer.extend(new_layers)
    with open(output_file, 'w') as fp:
        fp.write(pb.text_format.MessageToString(new_net))


def approx_lowrank_weights(orig_model, orig_weights, conf,
                           lowrank_model, lowrank_weights):
    orig_net = caffe.Net(orig_model, orig_weights, caffe.TEST)
    lowrank_net = caffe.Net(lowrank_model, orig_weights, caffe.TEST)
    print("Original model: " + orig_model + " weights: " + orig_weights)
    for layer_name in conf:
	print("Working on "+ layer_name) 
	W, b = [p.data for p in orig_net.params[layer_name]]
        v_weights, v_bias = \
            [p.data for p in lowrank_net.params[layer_name + '_v']]
        h_weights, h_bias = \
            [p.data for p in lowrank_net.params[layer_name + '_h']]
        # Set biases
        v_bias[...] = 0
        h_bias[...] = b.copy()
        # Get the shapes
        num_groups = v_weights.shape[0] // h_weights.shape[1]
        N, C, D, D = W.shape
        N = N // num_groups
        K = h_weights.shape[1]
        # SVD approximation
        for g in xrange(num_groups):
            W_ = W[N*g:N*(g+1)].transpose(1, 2, 3, 0).reshape((C*D, D*N))
            U, S, V = np.linalg.svd(W_)
            v = U[:, :K] * np.sqrt(S[:K])
            v = v[:, :K].reshape((C, D, 1, K)).transpose(3, 0, 1, 2)
            v_weights[K*g:K*(g+1)] = v.copy()
            h = V[:K, :] * np.sqrt(S)[:K, np.newaxis]
            h = h.reshape((K, 1, D, N)).transpose(3, 0, 1, 2)
            h_weights[N*g:N*(g+1)] = h.copy()
    lowrank_net.save(lowrank_weights)


def main(args):
    conf = load_config(args.config)
    # Make prototxt
    if args.save_model is None:
        print("Save model is none.")
	prefix, ext = osp.splitext(args.model)
        args.save_model = prefix + '_lowrank' + ext
    print("Making the lowrank model.")
    make_lowrank_model(args.model, conf, args.save_model)
    # Approximate conv weights
    if args.weights is None: return
    if args.save_weights is None:
        prefix, ext = osp.splitext(args.weights)
        args.save_weights = prefix + '_lowrank' + ext
    approx_lowrank_weights(args.model, args.weights, conf, args.save_model,
                           args.save_weights)


if __name__ == '__main__':
    parser = ArgumentParser(description="Low-rank approximation")
    parser.add_argument('--model', required=True,
        help="Prototxt of the original net")
    parser.add_argument('--config', required=True,
        help="JSON config file specifying the low-rank approximation")
    parser.add_argument('--weights',
        help="Caffemodel of the original net")
    parser.add_argument('--save_model',
        help="Path to the prototxt of the low-rank approximated net")
    parser.add_argument('--save_weights',
        help="Path to the caffemodel of the low-rank approximated net")
    args = parser.parse_args()
    main(args)
