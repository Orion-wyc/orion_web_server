# 指定 cmake版本
cmake_minimum_required(VERSION 3.10)

# 指定 project name
project(orion_web_server)

# 指定生成文件的输出目录, 并创建目录
set(PROJECT_OUTPUT_DIR ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR})
file(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_OUTPUT_DIR})

# 指定C/C++头文件目录
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(THIRD_PARTY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/3rdparty)

# 指定动态链接库
# target_link_libraries(${PROJECT_NAME} -ldl)

# 输出系统架构（x84-64）以及可执行文件的输出路径
message("-- system arch: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- output path: ${PROJECT_OUTPUT_DIR} ")

# 设置c++11编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -Wall")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/configs")
find_package(MySQL)

include_directories(${MYSQL_INCLUDE_DIR})

# 需要include的头文件
include_directories(
  ${PROJECT_INCLUDE_DIR}
  ${THIRD_PARTY_INCLUDE_DIR}
)

# 设置源文件代码
set(SOURCES
  ${PROJECT_SOURCE_DIR}/src/base/epoller.cpp
  ${PROJECT_SOURCE_DIR}/src/base/semaphore.cpp
  ${PROJECT_SOURCE_DIR}/src/base/stringbuffer.cpp
  ${PROJECT_SOURCE_DIR}/src/base/timer.cpp
  ${PROJECT_SOURCE_DIR}/src/http/httpconnection.cpp
  ${PROJECT_SOURCE_DIR}/src/http/httprequest.cpp
  ${PROJECT_SOURCE_DIR}/src/http/httpresponse.cpp
  ${PROJECT_SOURCE_DIR}/src/pool/sqlconnpool.cpp
  ${PROJECT_SOURCE_DIR}/src/pool/threadpool.cpp
  ${PROJECT_SOURCE_DIR}/src/server/server.cpp
  ${PROJECT_SOURCE_DIR}/src/utils/logger.cpp
  ${PROJECT_SOURCE_DIR}/src/utils/converter.cpp
  ${PROJECT_SOURCE_DIR}/src/main.cpp
)

# 全局变量的方式获取所有源文件
# file(GLOB SERVER_SRC
# ${PROJECT_SOURCE_DIR}/src/base/*.cpp
# ${PROJECT_SOURCE_DIR}/src/http/*.cpp
# ${PROJECT_SOURCE_DIR}/src/pool/*.cpp
# ${PROJECT_SOURCE_DIR}/src/server/*.cpp
# ${PROJECT_SOURCE_DIR}/src/utils/*.cpp
# ${PROJECT_SOURCE_DIR}/src/main.cpp
# )

# 创建可执行文件
add_executable(test_server ${SOURCES})

# 添加链接库
# target_link_libraries(test_server pthread mysqlclient)

# MYSQL
target_link_libraries(test_server pthread ${MYSQL_LIBRARIES})