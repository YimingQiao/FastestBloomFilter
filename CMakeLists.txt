cmake_minimum_required(VERSION 3.10)
project(FastestBloomFilter)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f")

include_directories(include)

# Create main benchmark executable
add_executable(main_benchmark src/main_benchmark.cpp)
