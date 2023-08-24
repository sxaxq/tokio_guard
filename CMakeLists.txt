cmake_minimum_required(VERSION 3.5)

project(ddos_guard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ddos_guard main.cpp)

find_package(Boost 1.70 REQUIRED COMPONENTS system) 
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(ddos_guard ${Boost_LIBRARIES})