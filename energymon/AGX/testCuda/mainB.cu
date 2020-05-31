// System Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <energymon/energymon-odroid.h>
//#include <energymon/energymon-default.h>
// Cuda Includes
//#include <cuda.h>
//#include <cublas.h>
//#include <cublas_v2.h>
//#include <cuda_runtime.h>

// auxiliary functions
#include "AuxFuncs.h"

// Macro to store elements in a linear space in row-major format
#define IDX2C(i, j, ld) (((i) * (ld)) + (j))
#define BLOCK_SIZE 512

__global__ void MultMVNaiveKernel(double *A, double *b, double *c, const int M, const int N){
	int row = threadIdx.x + blockIdx.x * blockDim.x;

	double sum = 0;
	if(row < M) {
		for(int i = 0; i < N; i++) {
			sum += b[i] * A[row * N + i];
		}
		c[row] = sum;
	}
}


int do_work(int argc, char ** argv) {
	int M, N;
	// init the seed with current local time
	srand(time(NULL));

	// Get M - N values from arguments
	if (argc == 3) {
		M = atoi(argv[1]);
		N = atoi(argv[2]);
	}
	else {
		fprintf(stderr, "Insufficient command line arguments!\n");
		fprintf(stderr, "USAGE: main <matrixHeight> <matrixWidth>\n");
		exit(-1);
	}

	double * h_A, * h_b, * h_c; // host copies of a, b, c
	double * d_A, * d_b, * d_c; // device copies of a, b, c
	d_A = d_b = d_c = 0;

	cudaEvent_t start, stop;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);

	// Allocate host memory for the matrix and the vectors
	((h_A = (double *) malloc(M * N * sizeof(double))) != 0) ?
	((h_b = (double *) malloc(N * sizeof(double))) != 0) ?
	((h_c = (double *) malloc(M * sizeof(double))) != 0) ?
	:
	_error_handler("host memory allocation error (C)\n") :
	_error_handler("host memory allocation error (B)\n") :
	_error_handler("host memory allocation error (A)\n") ;

	// Allocate device memory for the matrix and the vectors
	cudaMalloc((void **) &d_A, sizeof(double) * M * N);
	cudaMalloc((void **) &d_b, sizeof(double) * N);
	cudaMalloc((void **) &d_c, sizeof(double) * M);
	
	// Initialize matrix A and vector b with some values and also zero-ize c vector
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			h_A[i*N + j] = randDouble();
		}
	}

	for (int i = 0; i < N; i++) {
		h_b[i] = randDouble();
	}

	for (int i = 0; i < M; i++) {
		h_c[i] = 0;
	}

	cudaEventRecord(start);
	
	// Copy data from host to device
	cudaMemcpy(d_A, h_A, sizeof(double) * M * N, cudaMemcpyHostToDevice);
	cudaMemcpy(d_b, h_b, sizeof(double) * N, cudaMemcpyHostToDevice);
	//cudaMemcpy(d_c, h_c, sizeof(double) * M, cudaMemcpyHostToDevice);

	unsigned int numOfBlocks = M / BLOCK_SIZE + 1;	   

	// Run kernel and measure the time needed
	//MultMVNaiveKernel<<<numOfBlocks, BLOCK_SIZE>>>(d_A, d_b, d_c, M, N);
	// Get results from the device
	cudaMemcpy(h_c, d_c, M * sizeof(h_c[0]), cudaMemcpyDeviceToHost);
	
	cudaEventRecord(stop);
	cudaEventSynchronize(stop);

	float milliseconds = 0;
	cudaEventElapsedTime(&milliseconds, start, stop);
	fprintf(stdout, "Execution completed. Elapsed Time = %6.8f msecs\n", milliseconds);



	// Free host memory
	free(h_A); free(h_b); free(h_c);
	// Free GPU memory
	cudaFree(d_A); cudaFree(d_b); cudaFree(d_c);

	return EXIT_SUCCESS;
}

int main(int argc, char** argv){
	energymon em;
	uint64_t start_nj[4], end_nj[4];
	for(int i =0;i<4;i++){
		start_nj[i]=0;
		end_nj[i]=0;
	}

	// get the energymon instance and initialize
	//energymon_get_default(&em);
	energymon_get_odroid(&em);
	em.finit(&em);

	// profile application function
	uint64_t start = em.fread(&em, start_nj);
	
	do_work(argc, argv);
	uint64_t end = em.fread(&em, end_nj);
	printf("========= %lld\n", start);
	printf("========= %lld\n", end);

	printf("Total energy in mJ: %f\n", (end_nj[0]-start_nj[0])/1000000.0);
	printf("Total energy in mJ: %f\n", (end_nj[1]-start_nj[1])/1000000.0);
	printf("Total energy in mJ: %f\n", (end_nj[2]-start_nj[2])/1000000.0);
	printf("Total energy in mJ: %f\n", (end_nj[3]-start_nj[3])/1000000.0);

	// destroy the instance
	em.ffinish(&em);
	return 0;
}
