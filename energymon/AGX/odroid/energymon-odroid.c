/**
 * Energy reading for an ODROID with INA231 power sensors.
 *
 * @author Connor Imes
 * @date 2014-06-30
 */

#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "energymon.h"
#include "energymon-odroid.h"
#include "energymon-time-util.h"
#include "energymon-util.h"

#ifdef ENERGYMON_DEFAULT
#include "energymon-default.h"
int energymon_get_default(energymon* em) {
  return energymon_get_odroid(em);
}
#endif
///sys/bus/i2c/drivers/ina3221x/0-0041/iio:device1/in_power0_input"
//#define INA231_DIR "/sys/bus/i2c/drivers/INA231"
//#define INA231_FILE_TEMPLATE_POWER INA231_DIR"/%s/sensor_W"
//#define INA231_FILE_TEMPLATE_ENABLE INA231_DIR"/%s/enable"
//#define INA231_FILE_TEMPLATE_UPDATE_PERIOD INA231_DIR"/%s/update_period"
//#define INA231_DEFAULT_UPDATE_INTERVAL_US 263808

#define INA3221_DIR "/sys/bus/i2c/drivers/ina3221x"
#define INA3221_IN_POWER INA3221_DIR"/0-0041/iio_device/in_power2_input"
#define INA3221_GPU_POWER INA3221_DIR"/0-0040/iio_device/in_power0_input"
#define INA3221_CPU_POWER INA3221_DIR"/0-0040/iio_device/in_power1_input"
#define INA3221_DDR_POWER INA3221_DIR"/0-0041/iio_device/in_power1_input"
#define INA3221_DEFAULT_UPDATE_INTERVAL_US 110 //Default ina3221
//#define INA3221_DEFAULT_UPDATE_INTERVAL_US 1100 //Default ina3221
//#define INA3221_DEFAULT_UPDATE_INTERVAL_US 1000
//#define INA3221_DEFAULT_UPDATE_INTERVAL_US 263808


typedef struct energymon_odroid {
  // sensor update interval in microseconds
  unsigned long read_delay_us;
  // thread variables
  pthread_t thread;
  int poll_sensors;
  // total energy estimate
  uint64_t total_nj[4];
  // sensor file descriptors
  unsigned int count;
  int fds[4];
} energymon_odroid;


static inline long get_update_interval(char** sensors, unsigned int num) {
  unsigned long ret = INA3221_DEFAULT_UPDATE_INTERVAL_US;
  return ret;
}

int energymon_finish_odroid(energymon* em) {
  if (em == NULL || em->state == NULL) {
    errno = EINVAL;
    return -1;
  }

  int err_save = 0;
  unsigned int i;
  energymon_odroid* state = (energymon_odroid*) em->state;

  if (state->poll_sensors) {
    // stop sensors polling thread and cleanup
    state->poll_sensors = 0;
#ifndef __ANDROID__
    pthread_cancel(state->thread);
#endif
    err_save = pthread_join(state->thread, NULL);
  }

  // close individual sensor files
  for (i = 0; i < state->count; i++) {
    if (state->fds[i] > 0 && close(state->fds[i])) {
      err_save = err_save ? err_save : errno;
    }
  }
  free(em->state);
  em->state = NULL;
  errno = err_save;
  return errno ? -1 : 0;
}

static inline void free_sensor_directories(char** dirs, unsigned int n) {
  while (n > 0) {
    free(dirs[--n]);
  }
  free(dirs);
}


static inline char** get_sensor_directories(unsigned int* count) {
  unsigned int i;
  int err_save;
  struct dirent* entry;
  char** directories = NULL;
  *count = 4;
  errno = 0;
  DIR* sensors_dir = opendir(INA3221_DIR);
  if (sensors_dir == NULL) {
    return NULL;
  }
  directories = calloc(*count, sizeof(char*));
  directories[0]  = "0-0041";
  directories[1]  = "0-0040";
  if (closedir(sensors_dir)) {
    perror(INA3221_DIR);
  }
  return directories;
}
/**
 * pthread function to poll the sensors at regular intervals.
 */
static void* odroid_poll_sensors(void* args) {
  energymon_odroid* state = (energymon_odroid*) args;
  char cdata[8];
  double sum_w, gpu_w, cpu_w, ddr_w;
  unsigned int i;
  int64_t exec_us;
  int err_save;
  struct timespec ts;
  if (energymon_clock_gettime(&ts)) {
    // must be that CLOCK_MONOTONIC is not supported
    perror("odroid_poll_sensors");
    return (void*) NULL;
  }
  energymon_sleep_us(state->read_delay_us, &state->poll_sensors);
  while (state->poll_sensors) {
    // read individual sensors
      if (pread(state->fds[0], cdata, sizeof(cdata), 0) > 0) {
		  sum_w = strtod(cdata, NULL);
      }

	  if (pread(state->fds[1], cdata, sizeof(cdata), 0) > 0) {
		  gpu_w = strtod(cdata, NULL);
      }
	
	  if (pread(state->fds[2], cdata, sizeof(cdata), 0) > 0) {
		  cpu_w = strtod(cdata, NULL);
      }
	
	  if (pread(state->fds[3], cdata, sizeof(cdata), 0) > 0) {
		  ddr_w = strtod(cdata, NULL);
      }



    err_save = errno;
    exec_us = energymon_gettime_us(&ts);
    
	if (err_save) {
      errno = err_save;
      perror("odroid_poll_sensors: skipping power sensor reading");
	} else {
		//fprintf(stderr, ">>>> %lld \n", state->total_nj[0]);
		//fprintf(stderr, "\t %f \n", sum_w);
		//fprintf(stderr, "\t %d \n", exec_us);
		//fprintf(stderr, ">>>> %f  %f %d\n", state->total_nj[0], sum_w, exec_us);
		state->total_nj[0] += sum_w * exec_us;
		state->total_nj[1] += gpu_w * exec_us;
		state->total_nj[2] += cpu_w * exec_us;
		state->total_nj[3] += ddr_w * exec_us;
	}
    // sleep for the update interval of the sensors (minus most overhead)
    energymon_sleep_us(2 * state->read_delay_us - exec_us, &state->poll_sensors);
    errno = 0;
  }
  return (void*) NULL;
}

/**
 * Open all sensor files and start the thread to poll the sensors.
 */
int energymon_init_odroid(energymon* em) {
  if (em == NULL || em->state != NULL) {
    errno = EINVAL;
    return -1;
  }

  unsigned int i;
  char file[64];
  unsigned int count;

  // find the sensors
  char** sensor_dirs = get_sensor_directories(&count);
  if (count == 0) {
    perror("energymon_init_odroid: Failed to find power sensors: "INA3221_DIR);
    return -1;
  }
  size_t size = sizeof(energymon_odroid);
  energymon_odroid* state = calloc(1, size);
  if (state == NULL) {
    free_sensor_directories(sensor_dirs, count);
    return -1;
  }
  state->count = count;

  em->state = state;
  state->fds[0]=open(INA3221_IN_POWER, O_RDONLY);
  state->fds[1]=open(INA3221_GPU_POWER, O_RDONLY);
  state->fds[2]=open(INA3221_CPU_POWER, O_RDONLY);
  state->fds[3]=open(INA3221_DDR_POWER, O_RDONLY);

  // get the delay time between reads
  state->read_delay_us = get_update_interval(sensor_dirs, state->count);

  // start sensors polling thread
  state->poll_sensors = 1;
  errno = pthread_create(&state->thread, NULL, odroid_poll_sensors, state);
  if (errno) {
    energymon_finish_odroid(em);
    return -1;
  }

  return 0;
}

uint64_t energymon_read_total_odroid(const energymon* em, uint64_t* nj) {
  if (em == NULL || em->state == NULL) {
    errno = EINVAL;
    return 0;
  }
  nj[0]=((energymon_odroid*) em->state)->total_nj[0];
  nj[1]=((energymon_odroid*) em->state)->total_nj[1];
  nj[2]=((energymon_odroid*) em->state)->total_nj[2];
  nj[3]=((energymon_odroid*) em->state)->total_nj[3];
  return ((energymon_odroid*) em->state)->total_nj[0];
}

char* energymon_get_source_odroid(char* buffer, size_t n) {
  return energymon_strencpy(buffer, "ODROID INA3221 Power Sensors", n);
}

uint64_t energymon_get_interval_odroid(const energymon* em) {
  if (em == NULL || em->state == NULL) {
    errno = EINVAL;
    return 0;
  }
  return ((energymon_odroid*) em->state)->read_delay_us;
}

uint64_t energymon_get_precision_odroid(const energymon* em) {
  if (em == NULL || em->state == NULL) {
    errno = EINVAL;
    return 0;
  }
  // watts to 6 decimal places (microwatts) at refresh interval
  uint64_t prec = energymon_get_interval_odroid(em) / 1000000;
  return prec ? prec : 1;
}

int energymon_is_exclusive_odroid(void) {
  return 0;
}

int energymon_get_odroid(energymon* em) {
  if (em == NULL) {
    errno = EINVAL;
    return -1;
  }
  em->finit = &energymon_init_odroid;
  em->fread = &energymon_read_total_odroid;
  em->ffinish = &energymon_finish_odroid;
  em->fsource = &energymon_get_source_odroid;
  em->finterval = &energymon_get_interval_odroid;
  em->fprecision = &energymon_get_precision_odroid;
  em->fexclusive = &energymon_is_exclusive_odroid;
  em->state = NULL;
  return 0;
}
