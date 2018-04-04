cmake_minimum_required(VERSION 3.3.0)
project(myfs)

include_directories(${PROJECT_SOURCE_DIR} ${FUSE_INCLUDE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/release/)

set(CMAKE_C_COMPILER   "/usr/local/mpich/bin/mpicc")
set(CMAKE_CXX_COMPILER "/usr/local/mpich/bin/mpic++")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Wno-deprecated")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -fPIC")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})

find_package(FUSE REQUIRED)

add_subdirectory(util)
add_subdirectory(client)
add_subdirectory(datanode)

add_executable(myfs myfs.cpp)
target_link_libraries(myfs myfs-client myfs-datanode myfs-util ${FUSE_LIBRARIES})
