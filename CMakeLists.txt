cmake_minimum_required(VERSION 3.23)
project(taktile CXX)

cmake_policy(SET CMP0167 NEW)
#find_package(Boost 1.74 REQUIRED COMPONENTS system)
#find_package(Poco 1.11 REQUIRED COMPONENTS Foundation Net Util)
find_package(Poco 1.11 REQUIRED COMPONENTS Net)

add_library(${PROJECT_NAME} SHARED)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)