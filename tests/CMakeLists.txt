# Enable testing
enable_testing()

# Find GoogleTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add the test executable
add_executable(test_functions test_functions.cpp)
target_link_libraries(test_functions
  PRIVATE
    ${GTEST_LIBRARIES}
    pthread
    ${PROJECT_NAME}
)
add_test(NAME test_functions COMMAND test_functions)

add_executable(test_taktile test_taktile.cpp)
target_link_libraries(test_taktile
  PRIVATE
    fmt
    ${GTEST_LIBRARIES}
    pthread
    ${PROJECT_NAME}
)
add_test(NAME test_taktile COMMAND test_taktile)
