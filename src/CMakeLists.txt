target_sources(${PROJECT_NAME}
  PRIVATE
    constants.cpp
    functions.cpp
    taktile.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    fmt
    Boost::log
    Boost::log_setup
)
