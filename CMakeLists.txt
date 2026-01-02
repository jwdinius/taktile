cmake_minimum_required(VERSION 3.23)
project(taktile VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
cmake_policy(SET CMP0167 NEW)
option(${PROJECT_NAME}_ENABLE_TESTS "Build tests" ON)
option(${PROJECT_NAME}_BUILD_EXAMPLES "Build examples" ON)

find_package(Boost 1.74 REQUIRED COMPONENTS system log log_setup)
find_package(Poco 1.11 REQUIRED COMPONENTS Net XML)
find_package(simpleio 0.2 REQUIRED)

add_library(${PROJECT_NAME} SHARED)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_subdirectory(protobuf)

add_subdirectory(include)
add_subdirectory(src)

if(${PROJECT_NAME}_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
if(${PROJECT_NAME}_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Install
include(GNUInstallDirs)
install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  TARGETS
    ${PROJECT_NAME}
    takproto
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

# Export CMake package
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

include(InstallRequiredSystemLibraries)
execute_process(
  COMMAND dpkg --print-architecture
  OUTPUT_VARIABLE DEB_ARCH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# Required basic metadata
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-dev")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-dev_${PROJECT_VERSION}_${DEB_ARCH}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "josephwdinius@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "taktile: a C++ TAK client library")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VENDOR "Joe Dinius, Ph.D.")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Joe Dinius, Ph.D.")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/jwdinius/taktile.git")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-all-dev, libpoco-dev, libsimpleio-dev, libprotobuf-dev")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${DEB_ARCH}")

include(CPack)