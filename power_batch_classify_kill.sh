#batch_size=1
#
batch_array=(1 10 30 50 100 150)
mods=(Max-N Max-Q Max-P-All Max-P-ARM Max-P-Denver)
for batch in ${batch_array[@]}; do
	for (( mod=0; mod<=4; mod++ ))
	do
		sudo nvpmodel -m $mod
		#sudo ~/jetson_clocks.sh
		nohup ~/power-monitor.sh 0.01 &>> "logs/batch_150/power_classify_batch-$batch-150_${mods[$mod]}_dyn.log" &
		sleep 2
		./build/examples/cpp_classification/classification.bin models/bvlc_reference_caffenet/deploy.prototxt   models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel   data/ilsvrc12/imagenet_mean.binaryproto  data/ilsvrc12/synset_words.txt  $batch  /home/nvidia/Downloads/ILSVRC2012_img_train_t3/n02085620 &> /dev/null
		sleep 2
		sudo killall -9 power-monitor.sh
		sleep 10
	#echo "logs/batch_150/power_classify_batch-$batch-150_${mods[$mod]}_full.log"
	done
done

#sudo nvpmodel -m 0
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-N_full.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 1
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-Q_full.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 2
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-all_full.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 3
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-arm_full.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 4
#sudo ~/jetson_clocks.sh
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-denver_full.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 0
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-N_dyn.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 1
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-Q_dyn.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 2
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-all_dyn.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 3
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-arm_dyn.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
#sleep 10
#
#sudo nvpmodel -m 4
#nohup ~/power-monitor.sh 0.01 &>> logs/power_batch-classify-150_max-P-denver_dyn.log &
#sleep 2
#./batch_classify.sh
#sleep 2
#sudo killall -9 power-monitor.sh
