#python2 lowrank_approx.py   --model ../bvlc_reference_caffenet/deploy.prototxt   --config config.json   --save_model caffenet_lowrank_deploy.prototxt   --weights ../bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel --save_weights caffenet_lowrank.caffemodel
export PYTHONPATH=/home/nvidia/caffe-build/build-pycaffe/python/
python2 lowrank_approx.py   --model deploy.prototxt --save_model caffenet_lowrank_approx.prototxt   --config config.json  --weights bvlc_reference_caffenet.caffemodel --save_weights caffenet_lowrank.caffemodel
#python2 lowrank_approx.py   --model deploy.prototxt --save_model caffenet_lowrank_approx.prototxt   --config config.json

