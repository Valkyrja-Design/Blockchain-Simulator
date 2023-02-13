cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_COMPILER "usr/bin/g++.exe")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(BlockchainSimulator)

include_directories(./include)
file(GLOB SOURCES "./src/*.cpp")
add_executable(simulator ${SOURCES})

target_link_options(simulator PRIVATE -I ./src/include/)

