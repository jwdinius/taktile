# Specify the source files
target_sources(${PROJECT_NAME}
  PRIVATE
    constants.cpp
    functions.cpp  # List all your source files here
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Boost::log
    Boost::log_setup
    fmt::fmt
    Poco::XML
)
