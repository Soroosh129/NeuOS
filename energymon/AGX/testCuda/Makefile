#CFLAGS= -I/home/nvidia/GVirtuS/build/include  -L/home/nvidia/GVirtuS/build/lib -lgvirtus-frontend -L/home/nvidia/GVirtuS/build/lib/frontend  -lcudart
CFLAGS = -L/usr/local/lib -lenergymon-odroid -lpthread
CUDA_CFLAG = -lcudart

all:
	nvcc mainB.cu -o mainB $(CFLAGS) $(CUDA_CFLAG)
	nvcc monitor-thread.cu -o monitor-thread $(CFLAGS) $(CUDA_CFLAG)
	g++ -std=c++11 testIDLE.cpp -o testIDLE $(CFLAGS)
clean:
	rm -f mainA mainB mainC testIDLE
