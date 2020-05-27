CXX=g++
CFLAGS=-std=c++11 -I/home/nvidia/caffe-build-triangle/build-cudnn/include -I/home/nvidia/caffe-build-triangle/include -I/usr/local/cuda/include
LDFLAGS=`pkg-config --libs opencv`
LDFLAGS+=-L /usr/local/cuda/lib64 -lcuda -lcudnn -lcublas -L/home/nvidia/caffe-build-triangle/build-cudnn/lib -lcaffe /home/nvidia/caffe-build-triangle/build-cudnn/lib/libcaffeproto.a   -lglog -lopencv_core -lopencv_highgui -lopencv_imgproc -I /home/nvidia/caffe-build-triangle/build-cudnn/include -I /home/nvidia/caffe-build-triangle/include -I /usr/local/cuda/include -lenergymon-odroid -lboost_system  -lboost_thread -lboost_filesystem -lboost_chrono -lboost_date_time -lboost_atomic 
TARGET=classify.bin
all: $(TARGET)
clean:
	rm $(TARGET)
$(TARGET):
	$(CXX) $(CFLAGS) classification.cpp -o $(TARGET) $(LDFLAGS)
