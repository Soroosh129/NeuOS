#python lowrank_approx.py   --model /home/nvidia/caffe-build/models/bvlc_alexnet/deploy.prototxt   --config /home/nvidia/caffe-build/models/lowrank/config.json   --save_model alexnet_lowrank_deploy.prototxt --weights /home/nvidia/caffe-build/models/bvlc_alexnet/bvlc_alexnet.caffemodel --save_weights alexnet_lowrank.caffemodel
model="googlenet"
weight=""
proto=""
uncert="/home/nvidia/eval/uncertainty/"$model"-cudnn-uncertainty.txt"
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
	"vggnet") weight="/home/nvidia/caffe-build/models/vggnet/VGG_ILSVRC_19_layers.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/vggnet/VGG_ILSVRC_19_layers_deploy.prototxt" ;;
	"resnet") weight="/home/nvidia/caffe-build/models/resNet/ResNet-50-model.caffemodel" ;
		proto="/home/nvidia/caffe-build/models/resNet/ResNet-50-deploy.prototxt" ;;
	*) weight="NULL" ;
	proto="NULL";;
esac
python lowrank_approx_test.py   --model $proto   --config $model"-config.json"   --save_model $model"_lowrank_deploy.prototxt" --weights $weight --save_weights $model"_lowrank.caffemodel"

