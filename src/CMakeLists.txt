# Specify the source files
target_sources(${PROJECT_NAME}
  PRIVATE
    constants.cpp
    functions.cpp
    types.cpp
    url.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    fmt
    Boost::log
    Boost::log_setup
)
