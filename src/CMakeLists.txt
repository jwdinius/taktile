# Specify the source files
target_sources(${PROJECT_NAME}
  PRIVATE
    functions.cpp  # List all your source files here
)

#target_link_libraries(my_app PRIVATE Poco::Foundation Poco::Net Poco::Util)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    #Boost::system
    Poco::Net
)