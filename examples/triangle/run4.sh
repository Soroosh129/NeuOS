./parallel.sh "./run-cudnn.sh alexnet" 4 > alexnet-4.log
./parallel.sh "./run-cudnn.sh googlenet" 4 > googlenet-4.log
./parallel.sh "./run-cudnn.sh resnet" 4 > resnet-4.log
./parallel.sh "./run-cudnn.sh vggnet" 4 > vggnet-4.log
./parallel.sh "./run-cudnn.sh alexnet" 8 > alexnet-8.log
./parallel.sh "./run-cudnn.sh googlenet" 8 > googlenet-8.log
./parallel.sh "./run-cudnn.sh resnet" 8 > resnet-8.log
./parallel.sh "./run-cudnn.sh vggnet" 8 > vggnet-8.log
