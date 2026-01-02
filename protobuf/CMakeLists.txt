find_package(Protobuf 3.12 REQUIRED) 

set(ATAK_CIV_DIR $ENV{ATAK_CIV_DIR})

set(Protobuf_IMPORT_DIRS "${ATAK_CIV_DIR}/takproto")

set(PROTO_FILES
  "${ATAK_CIV_DIR}/takproto/contact.proto"
  "${ATAK_CIV_DIR}/takproto/cotevent.proto"
  "${ATAK_CIV_DIR}/takproto/detail.proto"
  "${ATAK_CIV_DIR}/takproto/group.proto"
  "${ATAK_CIV_DIR}/takproto/precisionlocation.proto"
  "${ATAK_CIV_DIR}/takproto/status.proto"
  "${ATAK_CIV_DIR}/takproto/takcontrol.proto"
  "${ATAK_CIV_DIR}/takproto/takmessage.proto"
  "${ATAK_CIV_DIR}/takproto/takv.proto"
  "${ATAK_CIV_DIR}/takproto/track.proto"
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/takproto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
  ${PROTO_FILES}
  PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/takproto"
  IMPORT_DIRS "${Protobuf_IMPORT_DIRS}"
)

add_library(takproto SHARED ${PROTO_SRCS} ${PROTO_HDRS})
add_library(${PROJECT_NAME}::takproto ALIAS takproto)
target_include_directories(takproto
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/takproto>
    ${Protobuf_INCLUDE_DIRS}
)
# Use LITE_RUNTIME; link the lite lib for smaller footprint.
target_link_libraries(takproto PUBLIC protobuf::libprotobuf-lite)

include(GNUInstallDirs)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/takproto/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/takproto"
  FILES_MATCHING PATTERN "*.h"
)