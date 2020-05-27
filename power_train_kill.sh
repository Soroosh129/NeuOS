##sudo nvpmodel -m 0
#nohup ../power-monitor.sh 0.01 &>> logs/power_classify_max-P-denver.log &
#sleep 3
##./examples/mnist/train_lenet.sh
#./classify.sh
#sleep 3
#sudo killall -9 power-monitor.sh

#sudo nvpmodel -m 0
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-N_full.log &
#sleep 5
#./examples/mnist/train_lenet.sh
#sleep 5
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 1
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-Q_full.log &
#sleep 5
#./examples/mnist/train_lenet.sh
#sleep 5
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 2
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-P-all_full.log &
#sleep 5
#./examples/mnist/train_lenet.sh
#sleep 5
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 3
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-P-arm_full.log &
#sleep 5
#./examples/mnist/train_lenet.sh
#sleep 5
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 4
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-P-denver_full.log &
#sleep 5
#./examples/mnist/train_lenet.sh
#sleep 5
#sudo killall -9 power-monitor.sh

sudo nvpmodel -m 0
nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-N_dyn.log &
sleep 5
./examples/mnist/train_lenet.sh
sleep 5
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 1
nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-Q_dyn.log &
sleep 5
./examples/mnist/train_lenet.sh
sleep 5
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 2
nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-P-all_dyn.log &
sleep 5
./examples/mnist/train_lenet.sh
sleep 5
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 3
nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-P-arm_dyn.log &
sleep 5
./examples/mnist/train_lenet.sh
sleep 5
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 4
nohup ~/power-monitor.sh 1 &>> logs/power_mnist_max-P-denver_dyn.log &
sleep 5
./examples/mnist/train_lenet.sh
sleep 5
sudo killall -9 power-monitor.sh
