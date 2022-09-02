CXX = g++
CFLAGS = -std=c++11 -O2 -Wall -g 
LINKS = -pthread

PROJECT_ROOT = ~/vscode_remote/orion_web_server
PROJECT_OUTPUT_DIR = $(PROJECT_ROOT)/test/bin
PROJECT_INCLUDE_DIR = $(PROJECT_ROOT)/include
THIRDPARTY_DIR = $(PROJECT_ROOT)/3rdparty



TARGET = test_threadpool
OBJS = $(PROJECT_ROOT)/src/base/stringbuffer.cpp \
       $(PROJECT_ROOT)/src/utils/logger.cpp \
			 $(PROJECT_ROOT)/src/pool/threadpool.cpp \
       $(PROJECT_ROOT)/test/test_threadpool/test_threadpool.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(PROJECT_OUTPUT_DIR)/$(TARGET) \
	$(LINKS) \
	-I $(PROJECT_INCLUDE_DIR) 

clean:
	rm -rf $(PROJECT_OUTPUT_DIR)/$(TARGET)