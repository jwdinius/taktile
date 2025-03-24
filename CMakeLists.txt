cmake_minimum_required(VERSION 3.23)
project(taktile CXX)

set(CMAKE_CXX_STANDARD 17)
cmake_policy(SET CMP0167 NEW)

# TODO: export a cmake package
add_subdirectory(submodules/simpleio)

find_package(Boost 1.74 REQUIRED COMPONENTS system log log_setup)
find_package(Poco 1.11 REQUIRED COMPONENTS Net XML)
find_package(fmt 8 REQUIRED)

add_library(${PROJECT_NAME} SHARED)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)
