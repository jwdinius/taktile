# Enable testing
enable_testing()

# Find GoogleTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
find_package(fmt 8 REQUIRED)
find_package(Microsoft.GSL 4 REQUIRED)

# Add the test executable
add_executable(test_functions test_functions.cpp)
target_link_libraries(test_functions
  PRIVATE
    ${GTEST_LIBRARIES}
    pthread
    fmt::fmt
    Microsoft.GSL::GSL
    ${PROJECT_NAME}
)
add_test(NAME test_functions COMMAND test_functions)
