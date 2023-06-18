cmake_minimum_required(VERSION 3.22)
project(OS_EX4)

set(CMAKE_CXX_STANDARD 11)

add_executable(OS_EX4 Utilities.cpp VirtualMemory.cpp SimpleTest.cpp PhysicalMemory.cpp)
