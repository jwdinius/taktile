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

add_executable(test_types test_types.cpp)
target_link_libraries(test_types
  PRIVATE
    fmt
    ${GTEST_LIBRARIES}
    pthread
    ${PROJECT_NAME}
)
add_test(NAME test_types COMMAND test_types)

add_executable(test_url test_url.cpp)
target_link_libraries(test_url
  PRIVATE
    ${GTEST_LIBRARIES}
    pthread
    ${PROJECT_NAME}
)
add_test(NAME test_url COMMAND test_url)
