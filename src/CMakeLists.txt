# Specify the source files
target_sources(${PROJECT_NAME}
  PRIVATE
    constants.cpp
    functions.cpp  # List all your source files here
)

#target_link_libraries(my_app PRIVATE Poco::Foundation Poco::Net Poco::Util)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Boost::system
    fmt::fmt
    Poco::Net
    Poco::XML
)