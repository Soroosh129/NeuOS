#!/bin/bash
# Need su previlege
# sudo su
echo "userspace" > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
#chmod a+rw /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
chmod a+rw /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed
#echo "userspace" > /sys/devices/system/cpu/cpufreq/policy1/scaling_governor
#chmod a+rw /sys/devices/system/cpu/cpufreq/policy1/scaling_governor
chmod a+rw /sys/devices/system/cpu/cpufreq/policy1/scaling_setspeed
echo "userspace" > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/governor
#chmod a+rw /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/governor
chmod a+rw /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/userspace/set_freq
chmod a+rw /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/max_freq
chmod a+rw /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/min_freq
chmod a+rw /sys/devices/gpu.0/force_idle
chmod a+rw /sys/devices/system/cpu/cpu1/online
chmod a+rw /sys/devices/system/cpu/cpu2/online
chmod a+rw /sys/devices/system/cpu/cpu3/online
chmod a+rw /sys/devices/system/cpu/cpu4/online
chmod a+rw /sys/devices/system/cpu/cpu5/online
chmod 777  /sys/kernel/debug/
chmod 777  /sys/kernel/debug/bpmp/
chmod 777  /sys/kernel/debug/bpmp/debug/
chmod 777  /sys/kernel/debug/bpmp/debug/clk/
chmod 777  /sys/kernel/debug/bpmp/debug/clk/emc/
chmod a+rw /sys/kernel/debug/bpmp/debug/clk/emc/rate
chmod a+rw /sys/kernel/debug/bpmp/debug/clk/emc/min_rate
chmod a+rw /sys/kernel/debug/bpmp/debug/clk/emc/max_rate
chmod a+rw /sys/devices/system/cpu/cpufreq/policy1/scaling_min_freq
chmod a+rw /sys/devices/system/cpu/cpufreq/policy1/scaling_max_freq
chmod a+rw /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq
chmod a+rw /sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq
