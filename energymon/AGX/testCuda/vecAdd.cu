// System Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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


int main(int argc, char ** argv) {
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

	// Copy data from host to device
	cudaMemcpy(d_A, h_A, sizeof(double) * M * N, cudaMemcpyHostToDevice);
	cudaMemcpy(d_b, h_b, sizeof(double) * N, cudaMemcpyHostToDevice);
	cudaMemcpy(d_c, h_c, sizeof(double) * M, cudaMemcpyHostToDevice);

	unsigned int numOfBlocks = M / BLOCK_SIZE + 1;	   

	// Run kernel and measure the time needed
	cudaEventRecord(start);
	//MultMVNaiveKernel<<<numOfBlocks, BLOCK_SIZE>>>(d_A, d_b, d_c, M, N);
	cudaEventRecord(stop);
	cudaEventSynchronize(stop);

	float milliseconds = 0;
	cudaEventElapsedTime(&milliseconds, start, stop);
	fprintf(stdout, "Execution completed. Elapsed Time = %6.8f msecs\n", milliseconds);

	// Get results from the device
	cudaMemcpy(h_c, d_c, M * sizeof(h_c[0]), cudaMemcpyDeviceToHost);
	
	// Free host memory
	free(h_A); free(h_b); free(h_c);
	// Free GPU memory
	cudaFree(d_A); cudaFree(d_b); cudaFree(d_c);

	return EXIT_SUCCESS;
}

