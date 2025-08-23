# 1) Find protobuf on your system (recommended)
find_package(Protobuf 3.12 REQUIRED) 

# Get environment variable value into a CMake variable
set(ATAK_CIV_DIR $ENV{ATAK_CIV_DIR})

# Make protoc find your imports like "detail.proto"
# (Add more dirs here if your protos live in multiple folders)
set(Protobuf_IMPORT_DIRS "${ATAK_CIV_DIR}/takproto")

# 2) List protos explicitly (better than GLOB for builds/CI)
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

# 3) Generate C++ sources/headers
# This macro comes from CMake's FindProtobuf.cmake
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
  ${PROTO_FILES}
  # Pass include paths to protoc (so "import" statements resolve)
  PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
  IMPORT_DIRS "${Protobuf_IMPORT_DIRS}"
)

# 4) Build a library from generated code
# Your protos use LITE_RUNTIME; link the lite lib for smaller footprint.
add_library(takproto SHARED ${PROTO_SRCS} ${PROTO_HDRS})
add_library(${PROJECT_NAME}::takproto ALIAS takproto)
target_include_directories(takproto
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}           # where generated headers go
    ${Protobuf_INCLUDE_DIRS}
)
# If some protos don’t use LITE_RUNTIME, link libprotobuf instead.
target_link_libraries(takproto PUBLIC protobuf::libprotobuf-lite)

# (Optional) installs
# install(TARGETS cot_protos cot_tool)