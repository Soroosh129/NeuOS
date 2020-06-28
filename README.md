# NeuOS
This is an implementation of the paper "A Latency-Predictable Multi-Dimensional Optimization Framework forDNN-driven Autonomous Systems". 

## Step 1
Install Energymon located in ``energymon/TX2`` for Jetson TX2 and in ``energymon/AGX`` for Jetson AGX Xavier:

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

## Step 2
**Before continueing, please make sure your cmake is at least version 3.12.2. You can check this by running:**
    
    cmake --version
You can download the latest version of CMake and compile it yourself from https://cmake.org/download/ .

You also need a fairly recent OpenCV (the one provided by NVIDIA in Jetpack doesn't cut it). Thankfully, there are very nice tutorials available for compiling your own OpenCV on the Jetson platforms at https://github.com/jetsonhacks/buildOpenCVXavier for AGX and at https://github.com/jetsonhacks/buildOpenCVTX2 for TX2 (JetsonHacks has all sorts of cool tutorials for these platforms).

We have tested NeuOS on Jetpack 4.3 (L4T 32.3.1) with CUDA 10.0. Future releases might not work. Try at your own risk. 

Please also make sure to install all the required dependencies from the original Caffe installation guide below before you compile Caffe or after you get errors :). The suggested dependencies can be installed by:

    sudo apt install libgflags-dev libgoogle-glog-dev protobuf-compiler liblmdb-dev libleveldb-dev libsnappy-dev libatlas-base-dev doxygen

Same goes for data, mean files, etc. which will come later (familierizing yourself with Caffe can help you better understand this setup process). 

To install NeuOS, create a ``build`` folder:

    mkdir build
    cd build
    cmake .. -DCUDA_USE_STATIC_CUDA_RUNTIME=OFF
    make -j6

To replicate the results in the paper, make sure CUDNN is installed and configured for Caffe (i.e., by uncommenting the ``USE_CUDNN :=1`` flag in ``Makefile.config``).


## Step 3
Download your desired weights or train them in the form of a ``.caffemodel`` and put them in the ``models`` folder. You also need the lowrank version of your model. We have provided a few python scripts in ``models/lowrank`` that convert the DNNs used in the paper to their lowrank version. However, extending this technique to other DNNs is very easy by just using one of these scripts as a template.

## Step 4
You need a hash table for DVFS configurations and their impact on performance and energy consumption. We have provided one for the Jetson TX2 located in the ``hashtables`` folder. We will add instruction on how to generate your own here soon. Since we are using only one approximation configuration (i.e., the lowrank) in this implementation (in addition to the baseline), no approximation hash table is required (i.e., the semantic is built into NeuOS).

You also need a file containing all the possible DVFS configurations for your system, including memory, etc. in a partially sorted format. We have provided an ``all-TX2.config`` for the Jetson TX2 and an ``all-AGX.config`` for Jetson AGX Xavier.

## Step 5
Go to the ``power-utility`` folder and run ``sudo ./enable-dvfs.sh`` for the appropriate platform.

## Step 6
We have created a custom ``classification.cpp``, along with its own makefile in ``examples/triangle``:

    cd examples/triangle
    make

Make sure to modify the ``Makefile`` to reflect the folder structure on your device. This will generate an executable called ``classify.bin``.

## Step 7
Because of the way NeuOS manipulates system files, the best way to run ``classify.bin`` is in the root environment, for example, by using ``sudo -i``.

In the same folder as ``classify.bin``, you find the ``run-cudnn.sh``, which has the following format (also note the preloading of your newly compiled Caffe, which is now the default TX2/AGX folder structure but should be changed according to your installation):

    ./classify.bin \
      $proto \
      $weight \
      /home/nvidia/caffe-build/data/ilsvrc12/imagenet_mean.binaryproto \
      /home/nvidia/caffe-build/data/ilsvrc12/synset_words.txt \
      /home/nvidia/caffe-build/examples/images/cat.jpg \ # The data you want to use
      15 \	         # Number of iterations (for research purposes and getting the average execution times)
      25 \           # Deadline
      5 \            # Initial slack (useless for now)
      DVFS configs \ # The index for possible DVFS configurations.
      $HASH_TABLE \     # The hash tables
      $lowrank_proto \  # The structure of the lowrank
      $lowrank_weight'  # The weights of the lowrank
    

You can re-use ``run-cudnn.sh`` or run using your own command. You can also use the ``parrallel.sh`` to run multiple instances of DNNs at the same time. Again please make sure the paths in the scripts reflect your own absolute paths. Also please make sure the DVFS configs point to the appropriate file (e.g., ``all-AGX.config`` for the AGX).

**Please see inside the power-utility folder for some useful scripts such as enabling the DVFS adjustments by adjusting the permissions**


## Special Instructions for AGX
Currently, there are some non-trivial differences between the implementation of AGX and TX2. The project by default works with TX2 out of the box. For AGX, I have put the different source files in a separate folder called ``AGX`` with the correct structure. If you are running and compiling NeuOS on the AGX Xavier, copy and replace the files in the AGX folder with the same structure into Caffe.

Please cite NeuOS in your publications if it helps your research (please also cite Caffe below):

    @inproceedings {soroush2020neuos,
    title = {NeuOS: A Latency-Predictable Multi-Dimensional Optimization Framework for DNN-driven Autonomous Systems},
    booktitle = {2020 {USENIX} Annual Technical Conference ({USENIX} {ATC} 20)},
    year = {2020},
    url = {https://www.usenix.org/conference/atc20/presentation/bateni},
    publisher = {{USENIX} Association},
    month = jul,
}

---

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
