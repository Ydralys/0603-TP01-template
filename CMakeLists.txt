cmake_minimum_required(VERSION 3.9)
project(BitStream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g -D_DEBUG -Wall -Wconversion -Wextra -Wsign-conversion")
#set(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG")

add_executable(BitStream-Exemple1 BitBase.h BitBlock.h BitStream.h Exemple1.cpp)
add_executable(BitStream-Exemple2 BitBase.h BitBlock.h BitStream.h Exemple2.cpp)
add_executable(BitStream-Exemple3 BitFloat.h Exemple3.cpp)
