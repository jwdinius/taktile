# Install tls certificates to the build directory under the certs directory
set(CERTS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/certs)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/certs_path.h"
  CONTENT "inline constexpr char CERTS_PATH[] = \"${CERTS_PATH}\";  //NOLINT"
)

add_executable(send send.cpp)
target_include_directories(send
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(send
  PRIVATE
    ${PROJECT_NAME}
    simpleio::transports::ip
)

add_executable(send_receive send_receive.cpp)
target_include_directories(send_receive
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(send_receive
  PRIVATE
    ${PROJECT_NAME}
    simpleio::transports::ip
)