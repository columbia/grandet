find_package (Protobuf REQUIRED)
protobuf_generate_cpp (PROTO_SRCS PROTO_HDRS grandet.proto)

add_library (proto ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(proto ${PROTOBUF_LIBRARY})
target_include_directories(proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
