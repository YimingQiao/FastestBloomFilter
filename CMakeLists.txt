cmake_minimum_required(VERSION 3.10)
project(FastestBloomFilter)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include_directories(include)

# Create main benchmark executable
add_executable(main_benchmark src/main_benchmark.cpp)
