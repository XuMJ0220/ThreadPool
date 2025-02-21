cmake_minimum_required(VERSION 3.0)
project(test)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)#头文件路径
include_directories(${CMAKE_SOURCE_DIR}/thridpatry)
link_directories(${CMAKE_SOURCE_DIR}/lib)#库文件路径

#add_subdirectory(test/muduotest)
#add_subdirectory(src/server)
