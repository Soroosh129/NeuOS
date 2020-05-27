#!/bin/bash
model=$1
echo $1
weight=""
proto=""
uncert="/home/nvidia/eval/uncertainty/"$model"-cudnn-uncertainty.txt"
lowrank_weight="/home/nvidia/caffe-build/models/lowrank/"$model"_lowrank.caffemodel";
lowrank_proto="/home/nvidia/caffe-build/models/lowrank/"$model"_lowrank_deploy.prototxt";
echo $uncert
case "$model" in 
	"googlenet") weight="/home/nvidia/caffe-build/models/bvlc_googlenet/bvlc_googlenet.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/bvlc_googlenet/deploy.prototxt" ;;
	"caffenet") weight="/home/nvidia/caffe-build/models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/bvlc_reference_caffenet/deploy.prototxt" ;;
	"alexnet") weight="/home/nvidia/caffe-build/models/bvlc_alexnet/bvlc_alexnet.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/bvlc_alexnet/deploy.prototxt" ;;
	"deepcomp") weight="/home/nvidia/caffe-build/models/Deep-Compression-AlexNet/alexnet.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/bvlc_alexnet/deploy.prototxt" ;;
	"squeezenet") weight="/home/nvidia/caffe-build/models/SqueezeNet_v1.0/squeezenet_v1.0.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/SqueezeNet_v1.0/deploy.prototxt" ;;
	"lowrank") weight="/home/nvidia/caffe-build/models/lowrank/caffenet_lowrank.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/lowrank/caffenet_lowrank_deploy.prototxt" ;;
	"vggnet") weight="/home/nvidia/caffe-build/models/vggnet/VGG_ILSVRC_19_layers.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/vggnet/VGG_ILSVRC_19_layers_deploy.prototxt" ;;
	"resnet") weight="/home/nvidia/caffe-build/models/resNet/ResNet-50-model.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/resNet/ResNet-50-deploy.prototxt" ;;
	*) weight="NULL" ;
		proto="NULL";;
esac
	
echo "loading weight: $weight"
echo "loading prototxt: $proto"

#./build/tools/caffe test --model=models/bvlc_alexnet/deploy.prototxt --weights=models/bvlc_alexnet/bvlc_alexnet.caffemodel --iterations=10 --gpu 0
LD_PRELOAD="/usr/local/lib/libenergymon-odroid.so /home/nvidia/caffe-build-triangle/build-cudnn/lib/libcaffe.so.1.0.0" \
  ./classify.bin \
  $proto \
  $weight \
  /home/nvidia/caffe-build/data/ilsvrc12/imagenet_mean.binaryproto \
  /home/nvidia/caffe-build/data/ilsvrc12/synset_words.txt \
  /home/nvidia/caffe-build/examples/images/cat.jpg \
  15 \
  25 \
  5 \
  /home/nvidia/eval/dnn-control/all.config \
  $uncert \
  $lowrank_proto \
  $lowrank_weight
  #/home/nvidia/caffe-build-dvfs/dvfs/med/Uncertainty.txt
