CXX = g++
CFLAGS = -std=c++11 -O2 -Wall -g 

PROJECT_ROOT = ~/vscode_remote/orion_web_server
PROJECT_OUTPUT_DIR = $(PROJECT_ROOT)/test/bin
PROJECT_INCLUDE_DIR = $(PROJECT_ROOT)/include

TARGET = test_sqlconnpool
OBJS = $(PROJECT_ROOT)/src/base/*.cpp $(PROJECT_ROOT)/src/pool/*.cpp \
			 $(PROJECT_ROOT)/test/test_sqlconnpool/test_sqlconnpool.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(PROJECT_OUTPUT_DIR)/$(TARGET)  -pthread -lmysqlclient -I $(PROJECT_INCLUDE_DIR)

clean:
	rm -rf ./bin/$(TARGET)