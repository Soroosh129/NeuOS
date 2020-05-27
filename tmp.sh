weight="models/bvlc_alexnet/bvlc_alexnet.caffemodel"
proto="models/tmp/tmp.prototxt"

./build/examples/cpp_classification/classification.bin \
  $proto \
  $weight \
  data/ilsvrc12/imagenet_mean.binaryproto \
  data/ilsvrc12/synset_words.txt \
  /home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620/n02085620_1152.JPEG


