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

//#define INA3221_DIR "/sys/bus/i2c/drivers/ina3221x"
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

class dvfs{
	public:
		dvfs(int a){
			int b =a;
			f_bigFreq_ = fopen(BIG_CLUSTER_FREQ,"w");
			if(f_bigFreq_ == NULL){
				cerr << "File open failure" << endl;
			}
			f_bigMax_ = fopen(BIG_CLUSTER_MAX, "w");
			f_bigMin_ = fopen(BIG_CLUSTER_MIN, "w");

			f_smallFreq_ = fopen(LITTLE_CLUSTER_FREQ,"w");
			f_smallMax_ = fopen(LITTLE_CLUSTER_MAX, "w");
			f_smallMin_ = fopen(LITTLE_CLUSTER_MIN, "w");
			
			for(int i =1;i<6;i++){
				char coreFile[256];
				sprintf(coreFile, CPU_ONLINE, i);
				f_cpuOnline_[i] = fopen(coreFile, "w");
			}
			
			f_gpuFreq_ = fopen(GPU_FREQ, "w");
			f_gpuOnline_ = fopen(GPU_ONLINE, "w");
			f_gpuMax_ = fopen(GPU_MAX, "w");
			f_gpuMin_ = fopen(GPU_MIN, "w");

			f_emcFreq_ = fopen(EMC_FREQ,"w");
			f_emcMax_ = fopen(EMC_MAX, "w");
			f_emcMin_ = fopen(EMC_MIN, "w");
		}

		~dvfs(){
			fclose(f_bigFreq_);
			fclose(f_bigMax_);
			fclose(f_bigMin_);
			fclose(f_smallFreq_);
			fclose(f_smallMax_);
			fclose(f_smallMin_);
			for(int i=1;i<6;i++)
				fclose(f_cpuOnline_[i]);
			fclose(f_gpuFreq_);
			fclose(f_gpuMin_);
			fclose(f_gpuMax_);
			fclose(f_gpuOnline_);
			fclose(f_emcFreq_);
			fclose(f_emcMax_);
			fclose(f_emcMin_);
		}
		int setBigClusterFreq(string freq){
			fputs(freq.c_str(), f_bigFreq_);
			fflush(f_bigFreq_);
			fputs(freq.c_str(), f_bigMin_);
			fflush(f_bigMin_);
			fputs(freq.c_str(), f_bigMax_);
			fflush(f_bigMax_);
			return 0;
		}
		int setLittleClusterFreq(string freq){
			fputs(freq.c_str(), f_smallFreq_);
			fflush(f_smallFreq_);
			fputs(freq.c_str(), f_smallMin_);
			fflush(f_smallMin_);
			fputs(freq.c_str(), f_smallMax_);
			fflush(f_smallMax_);
			return 0;
		}
		int setAllCoresOnline(string flags[], int len){
			// core 1-5
			// len =6
			for(int i =1;i<len;i++){
				fputs(flags[i].c_str(), f_cpuOnline_[i]);
				fflush(f_cpuOnline_[i]);
			}
			return 0;
		}
		int setGpuFreq(string gFreq){
			if(gFreq == "0"){
				fputs("1", f_gpuOnline_);
				fflush(f_gpuOnline_);
				// besides put GPU_idle to 1, also set its freq to lowest
				fputs("140250000", f_gpuFreq_);
				fflush(f_gpuFreq_);
				fputs("140250000", f_gpuMax_);
				fflush(f_gpuMax_);
				fputs("140250000", f_gpuMin_);
				fflush(f_gpuMin_);
				return 0;	
			}
			// if not 0, shouldn't be force_idle
			fputs("0", f_gpuOnline_);
			fflush(f_gpuOnline_);	
			fputs(gFreq.c_str(), f_gpuFreq_);
			fflush(f_gpuFreq_);	
			fputs(gFreq.c_str(), f_gpuMin_);
			fflush(f_gpuMin_);	
			fputs(gFreq.c_str(), f_gpuMax_);
			fflush(f_gpuMax_);	
			return 0;
		}
		int setEmcFreq(string emcFreq){
			const char* tmp = emcFreq.c_str();
			fputs(tmp, f_emcFreq_);
			fflush(f_emcFreq_);
			fputs(tmp, f_emcMin_);
			fflush(f_emcMin_);
			fputs(tmp, f_emcMax_);
			fflush(f_emcMax_);
			return 0;
		}
		int setConfig(string strConfig){
			stringstream stream(strConfig);
			vector<string> freq;
			string n;
			while(stream >> n){
				freq.push_back(n);
			}
			string bigFreq = freq[6];
			string littleFreq = freq[1];
			string gpuFreq = freq[9];
			string core[6]={"1",freq[7],freq[8], freq[3], freq[4], freq[5]};
			string emcFreq = freq[10];
			
			setAllCoresOnline(core, 6); // Must before set core frequence
			setBigClusterFreq(bigFreq);
			setLittleClusterFreq(littleFreq);
			setGpuFreq(gpuFreq);
			setEmcFreq(emcFreq);
			return 0;
		}
		int set_maxFreq(){
			string maxFreq = "2879 2035200 1 1 1 1 2035200 1 1 1300500000 1866000000";
			if(setConfig(maxFreq)!=0){
				cout << "Error set_maxFreq" << endl;
			}
			return 0;
		}
		int set_idle(){
			string idle = "22 345600 1 0 0 0 345600 0 0 0 204000000";
			cout << idle << endl;
			if(setConfig(idle)!=0){
				cout << "Error set_idle" << endl;
			}
			return 0;
		}
	private:
		FILE* f_bigFreq_;
		FILE* f_bigMax_;
		FILE* f_bigMin_;
		FILE* f_smallFreq_;
		FILE* f_smallMax_;
		FILE* f_smallMin_;

		FILE* f_cpuOnline_[6];

		FILE* f_gpuFreq_;
		FILE* f_gpuOnline_;
		FILE* f_gpuMax_;
		FILE* f_gpuMin_;

		FILE* f_emcFreq_;
		FILE* f_emcMax_;
		FILE* f_emcMin_;

};


int main(){
	dvfs dv(2);
	//dv.set_idle();
	dv.set_maxFreq();
	return 0;
}
