CXX = g++
CFLAGS = -std=c++11 -O2 -Wall -g 
LINKS = -pthread

PROJECT_ROOT = ~/vscode_remote/orion_web_server
PROJECT_OUTPUT_DIR = $(PROJECT_ROOT)/test/bin
PROJECT_INCLUDE_DIR = $(PROJECT_ROOT)/include


TARGET = test_blockqueue
OBJS = $(PROJECT_ROOT)/test/test_blockqueue/test_blockqueue.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(PROJECT_OUTPUT_DIR)/$(TARGET) \
	$(LINKS) \
	-I $(PROJECT_INCLUDE_DIR)

clean:
	rm -rf ./bin/$(TARGET)