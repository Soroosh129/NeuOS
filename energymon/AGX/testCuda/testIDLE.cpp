// System Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <energymon/energymon-odroid.h>
#include "set_config.h"

int main(int argc, char** argv){
	energymon em;
	uint64_t start_nj[4], end_nj[4];
	for(int i =0;i<4;i++){
		start_nj[i]=0;
		end_nj[i]=0;
	}

	ifstream infile("bounds.config");	
	string line = "0 345600 1 0 0 0 345600 0 0 0 204000000";
	//while(getline(infile, line)){
		//setConfig(line);
		energymon_get_odroid(&em);
		em.finit(&em);
		uint64_t start = em.fread(&em, start_nj);

		usleep(10000000);	

		uint64_t end = em.fread(&em, end_nj);
		printf("==== %s %f %f %f %f\n", 
				line.c_str(),
				(end_nj[0]-start_nj[0])/1000000.0, 
				(end_nj[1]-start_nj[1])/1000000.0, 
				(end_nj[2]-start_nj[2])/1000000.0, 
				(end_nj[3]-start_nj[3])/1000000.0);
		em.ffinish(&em);

	//}

	return 0;
}
