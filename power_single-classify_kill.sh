#!/bin/bash
mods=(Max-N Max-Q Max-P-All Max-P-ARM Max-P-Denver)
intervals=(0.1 0.001)
ms=(100 1)
net=(caffenet googlenet)
for (( n=0; n<=1; n++ ))
do
	for (( i=0; i<=1; i++ ))
	do
		for (( mod=0; mod<=4; mod++ ))
		do
			sudo nvpmodel -m $mod
			for (( boost=0; boost<=1; boost++ ))
			do
				if (( boost==0 )); then
					nohup ~/power-monitor.sh ${intervals[$i]} &>> "logs/${net[$n]}/power_classify_${net[$n]}_${intervals[$i]}_${mods[$mod]}_dyn.log" &
					sudo nohup ~/tegrastats ${ms[$i]} &>> "logs/${net[$n]}/power_classify_${net[$n]}_${intervals[$i]}_${mods[$mod]}_dyn.stat" &
				else
					sudo ~/jetson_clocks.sh
					nohup ~/power-monitor.sh ${intervals[$i]} &>> "logs/${net[$n]}/power_classify_${net[$n]}_${intervals[$i]}_${mods[$mod]}_full.log" &
					sudo nohup ~/tegrastats ${ms[$i]} &>> "logs/${net[$n]}/power_classify_${net[$n]}_${intervals[$i]}_${mods[$mod]}_full.stat" &
				fi
				sleep 2
				classify.sh ${net[$n]} &> /dev/null
				sleep 2
				sudo killall -9 tegrastats
				sudo killall -9 power-monitor.sh
				sleep 10
			done
		done
	done
done
