#./build/examples/cpp_classification/classification.bin \
#  models/bvlc_reference_caffenet/deploy.prototxt \
#  models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel \
#  data/ilsvrc12/imagenet_mean.binaryproto \
#  data/ilsvrc12/synset_words.txt \
#/home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620/n02085620_1152.JPEG
#  /home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620/n02085620_10074.JPEG
#  #examples/images/cat.jpg

./build/examples/cpp_classification/classification.bin \
  models/bvlc_reference_caffenet/deploy.prototxt \
  models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel \
  data/ilsvrc12/imagenet_mean.binaryproto \
  data/ilsvrc12/synset_words.txt \
  1 \
  /home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620
