#!/bin/bash
model=$1
echo $1
weight=""
proto=""
uncert="/home/nvidia/NeuOS/hashtables/TX2/"$model"-cudnn-HASHTABLE.txt"
lowrank_weight="/home/nvidia/NeuOS/models/lowrank/"$model"_lowrank.caffemodel";
lowrank_proto="/home/nvidia/NeuOS/models/lowrank/"$model"_lowrank_deploy.prototxt";
echo $uncert
case "$model" in 
	"googlenet") weight="/home/nvidia/NeuOS/models/bvlc_googlenet/bvlc_googlenet.caffemodel" ;
		proto="/home/nvidia/NeuOS/models/bvlc_googlenet/deploy.prototxt" ;;
	"caffenet") weight="/home/nvidia/NeuOS/models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel" ;
		proto="/home/nvidia/NeuOS/models/bvlc_reference_caffenet/deploy.prototxt" ;;
	"alexnet") weight="/home/nvidia/NeuOS/models/bvlc_alexnet/bvlc_alexnet.caffemodel" ;
		proto="/home/nvidia/NeuOS/models/bvlc_alexnet/deploy.prototxt" ;;
	"deepcomp") weight="/home/nvidia/NeuOS/models/Deep-Compression-AlexNet/alexnet.caffemodel" ;
		proto="/home/nvidia/NeuOS/models/bvlc_alexnet/deploy.prototxt" ;;
	"squeezenet") weight="/home/nvidia/NeuOS/models/SqueezeNet_v1.0/squeezenet_v1.0.caffemodel" ;
		proto="/home/nvidia/NeuOS/models/SqueezeNet_v1.0/deploy.prototxt" ;;
	"lowrank") weight="/home/nvidia/NeuOS/models/lowrank/caffenet_lowrank.caffemodel" ;
		proto="/home/nvidia/NeuOS/models/lowrank/caffenet_lowrank_deploy.prototxt" ;;
	"vggnet") weight="/home/nvidia/NeuOS/models/vggnet/VGG_ILSVRC_19_layers.caffemodel" ;
		proto="/home/nvidia/NeuOS/models/vggnet/VGG_ILSVRC_19_layers_deploy.prototxt" ;;
	"resnet") weight="/home/nvidia/NeuOS/models/resNet/ResNet-50-model.caffemodel" ;
		proto="/home/nvidia/NeuOS/models/resNet/ResNet-50-deploy.prototxt" ;;
	*) weight="NULL" ;
		proto="NULL";;
esac
	
echo "loading weight: $weight"
echo "loading prototxt: $proto"

#./build/tools/caffe test --model=models/bvlc_alexnet/deploy.prototxt --weights=models/bvlc_alexnet/bvlc_alexnet.caffemodel --iterations=10 --gpu 0
LD_PRELOAD="/usr/local/lib/libenergymon-odroid.so /home/nvidia/NeuOS/build/lib/libcaffe.so.1.0.0" \
  ./classify.bin \
  $proto \
  $weight \
  /home/nvidia/NeuOS/data/ilsvrc12/imagenet_mean.binaryproto \
  /home/nvidia/NeuOS/data/ilsvrc12/synset_words.txt \
  /home/nvidia/NeuOS/examples/images/cat.jpg \
  15 \
  25 \
  5 \
  all-TX2.config \
  $uncert \
  $lowrank_proto \
  $lowrank_weight
  #/home/nvidia/NeuOS-dvfs/dvfs/med/Uncertainty.txt
