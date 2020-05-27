#!/bin/bash
model=$1
weight=""
proto=""
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

	*) weight="NULL" ;
		proto="NULL";;
esac
	
echo "loading weight: $weight"
echo "loading prototxt: $proto"

#./build/tools/caffe test --model=models/bvlc_alexnet/deploy.prototxt --weights=models/bvlc_alexnet/bvlc_alexnet.caffemodel --iterations=10 --gpu 0

LD_PRELOAD=/usr/local/lib/libenergymon-odroid.so /home/nvidia/caffe-build-dvfs/build-gpu/examples/cpp_classification/classification.bin \
  $proto \
  $weight \
  /home/nvidia/caffe-build/data/ilsvrc12/imagenet_mean.binaryproto \
  /home/nvidia/caffe-build/data/ilsvrc12/synset_words.txt \
  /home/nvidia/caffe-build/examples/images/cat.jpg\
  5\
  /home/nvidia/eval/dnn-control/all.config \
  /home/nvidia/caffe-build-dvfs/dvfs/med/Uncertainty.txt
  #/home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620/n02085620_1152.JPEG


#./build/examples/cpp_classification/classification.bin \
#  models/bvlc_reference_caffenet/deploy.prototxt \
#  models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel \
#  data/ilsvrc12/imagenet_mean.binaryproto \
#  data/ilsvrc12/synset_words.txt \
#  /home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620/n02085620_1152.JPEG
#  /home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620/n02085620_10074.JPEG
#  #examples/images/cat.jpg

# Batch classifiction
# arg[5]: batch
# arg[6]: directory
#./build/examples/cpp_classification/classification.bin \
#  models/bvlc_reference_caffenet/deploy.prototxt \
#  models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel \
#  data/ilsvrc12/imagenet_mean.binaryproto \
#  data/ilsvrc12/synset_words.txt \
#  3 \
#  /home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620

#./build/examples/cpp_classification/classification.bin \
#  models/bvlc_googlenet/deploy.prototxt \
#  models/bvlc_googlenet/bvlc_googlenet.caffemodel \
#  data/ilsvrc12/imagenet_mean.binaryproto \
#  data/ilsvrc12/synset_words.txt \
#  /home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620/n02085620_1152.JPEG

