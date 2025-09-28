cmake_minimum_required(VERSION 3.23)
project(taktile CXX)

set(CMAKE_CXX_STANDARD 17)
cmake_policy(SET CMP0167 NEW)

find_package(Boost 1.74 REQUIRED COMPONENTS system log log_setup)
find_package(Poco 1.11 REQUIRED COMPONENTS Net XML)
#find_package(simpleio 0 REQUIRED)

add_library(${PROJECT_NAME} SHARED)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_subdirectory(protobuf)
add_subdirectory(submodules/simpleio)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)
