#include <iostream>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <fstream>
#include <utility>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;


#define BIG_CLUSTER_GOVERNOR "/sys/devices/system/cpu/cpufreq/policy1/scaling_governor"
#define BIG_CLUSTER_FREQ "/sys/devices/system/cpu/cpufreq/policy1/scaling_setspeed"
#define BIG_CLUSTER_MAX "/sys/devices/system/cpu/cpufreq/policy1/scaling_max_freq"
#define BIG_CLUSTER_MIN "/sys/devices/system/cpu/cpufreq/policy1/scaling_min_freq"

#define LITTLE_CLUSTER_GOVERNOR "/sys/devices/system/cpu/cpufreq/policy0/scaling_governor"
#define LITTLE_CLUSTER_FREQ "/sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed"
#define LITTLE_CLUSTER_MAX "/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq"
#define LITTLE_CLUSTER_MIN "/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq"


#define GPU_GOVERNOR "/sys/devices/17000000.gp10b/devfreq/17000000.gp10b/governor"
#define GPU_FREQ "/sys/devices/17000000.gp10b/devfreq/17000000.gp10b/userspace/set_freq"
#define GPU_ONLINE "/sys/devices/gpu.0/force_idle"
#define GPU_MAX "/sys/devices/17000000.gp10b/devfreq/17000000.gp10b/max_freq"
#define GPU_MIN "/sys/devices/17000000.gp10b/devfreq/17000000.gp10b/min_freq"

#define CPU_ONLINE "/sys/devices/system/cpu/cpu%d/online"
#define EMC_FREQ "/sys/kernel/debug/bpmp/debug/clk/emc/rate"
#define EMC_MIN "/sys/kernel/debug/bpmp/debug/clk/emc/min_rate"
#define EMC_MAX "/sys/kernel/debug/bpmp/debug/clk/emc/max_rate"


int setBigClusterFreq(uint64_t freq){
	FILE* f_freq = fopen(BIG_CLUSTER_FREQ,"w");
	FILE* f_max = fopen(BIG_CLUSTER_MAX, "w");
	FILE* f_min = fopen(BIG_CLUSTER_MIN, "w");
	if(f_freq == NULL || f_max ==NULL || f_min ==NULL){
		fprintf(stderr, "File open failure\n");
		return -1;
	}
	char buff[128];
	sprintf(buff, "%" PRIu64, freq);
	fputs(buff, f_freq);
	fputs(buff, f_min);
	fputs(buff, f_max);
	fclose(f_freq);
	fclose(f_min);
	fclose(f_max);
	return 0;
}
int setLittleClusterFreq(uint64_t freq){
	FILE* f_freq = fopen(LITTLE_CLUSTER_FREQ,"w");
	FILE* f_max = fopen(LITTLE_CLUSTER_MAX, "w");
	FILE* f_min = fopen(LITTLE_CLUSTER_MIN, "w");
	if(f_freq == NULL || f_max ==NULL || f_min ==NULL){
		fprintf(stderr, "File open failure\n");
		return -1;
	}
	char buff[128];
	sprintf(buff, "%" PRIu64, freq);
	fputs(buff, f_freq);
	fputs(buff, f_min);
	fputs(buff, f_max);
	fclose(f_freq);
	fclose(f_min);
	fclose(f_max);
	return 0;
}

int setSmallCoresOnline(int mask[]){
	int smallIdx[]={0,3,4,5};
	int i;
	for(i=1;i<4;i++){
		char buffer[128];
		sprintf(buffer, CPU_ONLINE, smallIdx[i]);
		FILE* f_online=fopen(buffer,"w");
		if(f_online==NULL){
			fprintf(stderr, "Failed to set small cores online\n");
			return -1;
		}
		if(mask[i]==0){
			fputs("0", f_online);	
		}else{
			fputs("1", f_online);
		}
		fclose(f_online);
	}
	return 0;
}

int setSmallCoresOnline(int cores){
	int smallIdx[]={0,5,4,3};
	int i;
	for(i=1;i<4;i++){
		char buffer[128];
		sprintf(buffer, CPU_ONLINE, smallIdx[i]);
		FILE* f_online=fopen(buffer,"w");
		if(f_online==NULL){
			fprintf(stderr, "Failed to set small cores online\n");
			return -1;
		}
		if(i>=cores){
			fputs("0", f_online);	
		}else{
			fputs("1", f_online);
		}
		fclose(f_online);
	}
	return 0;
}

int setBigCoresOnline(int mask[]){
	int bigIdx[]={1,2};
	int i;
	for(i=0;i<2;i++){
		char buffer[128];
		sprintf(buffer, CPU_ONLINE, bigIdx[i]);
		FILE* f_online=fopen(buffer,"w");
		if(f_online==NULL){
			fprintf(stderr, "Failed to set small cores online\n");
			return -1;
		}
		if(mask[i]==0){
			fputs("0", f_online);	
		}else{
			fputs("1", f_online);
		}
		fclose(f_online);
	}
	return 0;
}

int setBigCoresOnline(int cores){
	int bigIdx[]={2,1};
	int i;
	for(i=0;i<2;i++){
		char buffer[128];
		sprintf(buffer, CPU_ONLINE, bigIdx[i]);
		FILE* f_online=fopen(buffer,"w");
		if(f_online==NULL){
			fprintf(stderr, "Failed to set small cores online\n");
			return -1;
		}
		if(i>=cores){
			fputs("0", f_online);	
		}else{
			fputs("1", f_online);
		}
		fclose(f_online);
	}
	return 0;
}
int setGpuFreq(uint64_t gFreq){
	FILE* f_freq = fopen(GPU_FREQ,"w");
	FILE* f_max = fopen(GPU_MAX, "w");
	FILE* f_min = fopen(GPU_MIN, "w");
	FILE* f_idle = fopen(GPU_ONLINE, "w");
	if(f_freq == NULL || f_max ==NULL || f_idle==NULL||f_min ==NULL){
		fprintf(stderr,  "File open failure\n");
		return -1;
	}
	if(gFreq == 0){
		fputs("1", f_idle);
		fputs("140250000", f_freq);
		fputs("140250000", f_max);
		fputs("140250000", f_min);
		fclose(f_idle);
		fclose(f_freq);
		fclose(f_max);
		fclose(f_min);
		return 0;	
	}
	char buff[128];
	sprintf(buff, "%" PRIu64, gFreq);
	fputs("0", f_idle);
	fputs(buff, f_freq);
	fputs(buff, f_min);
	fputs(buff, f_max);
	fclose(f_freq);
	fclose(f_min);
	fclose(f_max);
	return 0;
}
int setEmcFreq(uint64_t emcFreq){
	FILE* f_freq = fopen(EMC_FREQ,"w");
	FILE* f_max = fopen(EMC_MAX, "w");
	FILE* f_min = fopen(EMC_MIN, "w");
	if(f_freq == NULL || f_max ==NULL || f_min ==NULL){
		fprintf(stderr, "File open failure\n");
		return -1;
	}
	char buff[128];
	sprintf(buff, "%" PRIu64, emcFreq);
	fputs(buff, f_freq);
	fputs(buff, f_min);
	fputs(buff, f_max);
	fclose(f_freq);
	fclose(f_min);
	fclose(f_max);
	return 0;
}
