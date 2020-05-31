#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
int main(int argc, char** argv)
{
	int interval = 1000;
	FILE* pfile;
	if (argc == 3){
		interval = atoi(argv[1]);
		pfile=fopen(argv[2], "a");
	}
	if (argc > 3) {
		fprintf(stderr, "usage: %s <interval in ms> <file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int f_IN, f_GPU, f_CPU, f_SOC, f_DDR; 
	if ((f_IN = open("/sys/bus/i2c/drivers/ina3221x/0-0041/iio:device1/in_power0_input", O_RDONLY)) == -1) {
		exit(EXIT_FAILURE);
	}
	if ((f_GPU = open("/sys/bus/i2c/drivers/ina3221x/0-0040/iio:device0/in_power0_input", O_RDONLY)) == -1) {
		exit(EXIT_FAILURE);
	}
	if ((f_CPU = open("/sys/bus/i2c/drivers/ina3221x/0-0041/iio:device1/in_power1_input", O_RDONLY)) == -1) {
		exit(EXIT_FAILURE);
	}
	if ((f_DDR = open("/sys/bus/i2c/drivers/ina3221x/0-0041/iio:device1/in_power2_input", O_RDONLY)) == -1) {
		exit(EXIT_FAILURE);
	}
	if ((f_SOC = open("/sys/bus/i2c/drivers/ina3221x/0-0040/iio:device0/in_power1_input", O_RDONLY)) == -1) {
		exit(EXIT_FAILURE);
	}


	while(1){	
		//if(){
		struct timeval tv;
		gettimeofday(&tv, NULL);
		unsigned long timestamp = 1000000 * tv.tv_sec + tv.tv_usec;
		//unsigned long last_time = timestamp;

		char buffer[128] = {0};
		char *out = malloc(256 * sizeof(char));	
		int num;
		char* timebuf = malloc(256*sizeof(char));
		sprintf(timebuf, "[%ld] VDD_IN ", timestamp);
		strcat(out, timebuf);
		num = pread(f_IN, buffer, 127, 0);
		strncat(out, buffer, num);
		
		sprintf(timebuf, "[%ld] VDD_GPU ", timestamp);
		strcat(out, timebuf);
		num =  pread(f_GPU, buffer, 127, 0);		
		strncat(out, buffer, num);
		
		sprintf(timebuf, "[%ld] VDD_CPU ", timestamp);
		strcat(out, timebuf);
		num =  pread(f_CPU, buffer, 127, 0);		
		strncat(out, buffer, num);
		
		sprintf(timebuf, "[%ld] VDD_DDR ", timestamp);
		strcat(out, timebuf);
		num =  pread(f_DDR, buffer, 127, 0);		
		strncat(out, buffer, num);
		
		//printf("%s", out);
		fprintf(pfile, "%s", out);
		usleep(1000 * interval);
		//}
	}

	exit(EXIT_SUCCESS);
}
