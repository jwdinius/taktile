# Enable testing
enable_testing()

# Find GoogleTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add the test executable
add_executable(test_functions test_functions.cpp)  # Adjust the path to your test file
target_link_libraries(test_functions ${GTEST_LIBRARIES} pthread fmt::fmt Poco::XML taktile)

add_test(NAME test_functions COMMAND test_functions)