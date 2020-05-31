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
#define CLUSTER_GOVERNOR "/sys/devices/system/cpu/cpufreq/policy0/scaling_governor"
#define CLUSTER_FREQ "/sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed"
#define CLUSTER_MAX "/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq"
#define CLUSTER_MIN "/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq"


#define GPU_GOVERNOR "/sys/devices/17000000.gv11b/devfreq/17000000.gv11b/governor"
#define GPU_FREQ "/sys/devices/17000000.gv11b/devfreq/17000000.gv11b/userspace/set_freq"
#define GPU_ONLINE "/sys/devices/gpu.0/force_idle"
#define GPU_MAX "/sys/devices/17000000.gv11b/devfreq/17000000.gv11b/max_freq"
#define GPU_MIN "/sys/devices/17000000.gv11b/devfreq/17000000.gv11b/min_freq"

#define CPU_ONLINE "/sys/devices/system/cpu/cpu%d/online"
#define EMC_FREQ "/sys/kernel/debug/bpmp/debug/clk/emc/rate"
#define EMC_MIN "/sys/kernel/debug/bpmp/debug/clk/emc/min_rate"
#define EMC_MAX "/sys/kernel/debug/bpmp/debug/clk/emc/max_rate"

class dvfs{
	public:
		dvfs(int a){
			int b =a;
			f_Freq_ = fopen(CLUSTER_FREQ,"w");
			if(f_Freq_ == NULL){
				cerr << "File open failure" << endl;
			}
			f_Max_ = fopen(CLUSTER_MAX, "w");
			f_Min_ = fopen(CLUSTER_MIN, "w");

			
			for(int i =1;i<8;i++){
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
			fclose(f_Freq_);
			fclose(f_Max_);
			fclose(f_Min_);
			for(int i=1;i<8;i++)
				fclose(f_cpuOnline_[i]);
			fclose(f_gpuFreq_);
			fclose(f_gpuMin_);
			fclose(f_gpuMax_);
			fclose(f_gpuOnline_);
			fclose(f_emcFreq_);
			fclose(f_emcMax_);
			fclose(f_emcMin_);
		}
		int setClusterFreq(string freq){
			fputs(freq.c_str(), f_Freq_);
			fflush(f_Freq_);
			fputs(freq.c_str(), f_Min_);
			fflush(f_Min_);
			fputs(freq.c_str(), f_Max_);
			fflush(f_Max_);
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
			string CPUfreq = freq[1];
			string gpuFreq = freq[19];
			string core[8]={"1",freq[3],freq[4], freq[5], freq[6], freq[7], freq[8], freq[9]};
			string emcFreq = freq[11];
			
			setAllCoresOnline(core, 8); // Must before set core frequence
			setClusterFreq(CPUfreq);
			setGpuFreq(gpuFreq);
			setEmcFreq(emcFreq);
			return 0;
		}
		int set_maxFreq(){
			string maxFreq = "51967 2265600 1 1 1 1 1 1 1 1 1377000000 2133";
			if(setConfig(maxFreq)!=0){
				cout << "Error set_maxFreq" << endl;
			}
			return 0;
		}
		int set_idle(){
			string idle = "0 115200 1 0 0 0 0 0 0 0 114750000 2133";
			cout << idle << endl;
			if(setConfig(idle)!=0){
				cout << "Error set_idle" << endl;
			}
			return 0;
		}
	private:
		FILE* f_Freq_;
		FILE* f_Max_;
		FILE* f_Min_;

		FILE* f_cpuOnline_[8];

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
