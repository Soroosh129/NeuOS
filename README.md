# NeuOS
This is an implementation of the paper "A Latency-Predictable Multi-Dimensional Optimization Framework forDNN-driven Autonomous Systems". You can follow the normal Caffe build procedure. However, you would need a modified version of energymon for both the NVIDIA AGX Xavier and NVIDIA Jetson TX2. A repository with our version will follow up soon. You also need lowrank versions of a model. We have included the scripts for AlexNet, CaffeNet, GoogleNet, ResNet, and VGGNet to convert your trained or downloaded ".caffemodel" files to the lowrank version. Finally, you need a hash table for DVFS configurations. The execution format would then be:

    './classify.bin \
      $proto \
      $weight \
      /home/nvidia/caffe-build/data/ilsvrc12/imagenet_mean.binaryproto \
      /home/nvidia/caffe-build/data/ilsvrc12/synset_words.txt \
      /home/nvidia/caffe-build/examples/images/cat.jpg \
      15 \
      25 \
      5 \
      DVFS configs \ # The index for possible DVFS configurations.
      $HASH_TABLE \
      $lowrank_proto \
      $lowrank_weight'
    
Further instructions will soon follow.


# Caffe

[![Build Status](https://travis-ci.org/BVLC/caffe.svg?branch=master)](https://travis-ci.org/BVLC/caffe)
[![License](https://img.shields.io/badge/license-BSD-blue.svg)](LICENSE)

Caffe is a deep learning framework made with expression, speed, and modularity in mind.
It is developed by Berkeley AI Research ([BAIR](http://bair.berkeley.edu))/The Berkeley Vision and Learning Center (BVLC) and community contributors.

Check out the [project site](http://caffe.berkeleyvision.org) for all the details like

- [DIY Deep Learning for Vision with Caffe](https://docs.google.com/presentation/d/1UeKXVgRvvxg9OUdh_UiC5G71UMscNPlvArsWER41PsU/edit#slide=id.p)
- [Tutorial Documentation](http://caffe.berkeleyvision.org/tutorial/)
- [BAIR reference models](http://caffe.berkeleyvision.org/model_zoo.html) and the [community model zoo](https://github.com/BVLC/caffe/wiki/Model-Zoo)
- [Installation instructions](http://caffe.berkeleyvision.org/installation.html)

and step-by-step examples.

[![Join the chat at https://gitter.im/BVLC/caffe](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/BVLC/caffe?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Please join the [caffe-users group](https://groups.google.com/forum/#!forum/caffe-users) or [gitter chat](https://gitter.im/BVLC/caffe) to ask questions and talk about methods and models.
Framework development discussions and thorough bug reports are collected on [Issues](https://github.com/BVLC/caffe/issues).

Happy brewing!

## License and Citation

Caffe is released under the [BSD 2-Clause license](https://github.com/BVLC/caffe/blob/master/LICENSE).
The BAIR/BVLC reference models are released for unrestricted use.

Please cite Caffe in your publications if it helps your research:

    @article{jia2014caffe,
      Author = {Jia, Yangqing and Shelhamer, Evan and Donahue, Jeff and Karayev, Sergey and Long, Jonathan and Girshick, Ross and Guadarrama, Sergio and Darrell, Trevor},
      Journal = {arXiv preprint arXiv:1408.5093},
      Title = {Caffe: Convolutional Architecture for Fast Feature Embedding},
      Year = {2014}
    }
