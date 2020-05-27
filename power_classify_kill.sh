sudo nvpmodel -m 0
sudo ~/jetson_clocks.sh
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-N_full.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 1
sudo ~/jetson_clocks.sh
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-Q_full.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 2
sudo ~/jetson_clocks.sh
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-all_full.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 3
sudo ~/jetson_clocks.sh
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-arm_full.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 4
sudo ~/jetson_clocks.sh
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-denver_full.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 0
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-N_dyn.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 1
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-Q_dyn.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 2
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-all_dyn.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 3
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-arm_dyn.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
sleep 10

sudo nvpmodel -m 4
nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-denver_dyn.log &
sleep 2
./batch_classify.sh
sleep 2
sudo killall -9 power-monitor.sh
