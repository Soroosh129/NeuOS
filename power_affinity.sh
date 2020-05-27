#!/bin/bash
log="logs/layers/classify_single_nosleep_mod1max_core4+5_alexnet.log"
out="logs/layers/classify_single_nosleep_mod1max_core4+5_alexnet.out"
if [ -f $log ]
then
	rm $log
fi
nohup taskset 0x20 ~/power-monitor.bin 1 $log &
sleep 2
sudo taskset 0x10 ./classify.sh alexnet &> $out
sleep 2
sudo killall -9 power-monitor.bin
