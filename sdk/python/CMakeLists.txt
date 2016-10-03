add_custom_command(OUTPUT grandet.py COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/grandet.py ${CMAKE_CURRENT_BINARY_DIR} MAIN_DEPENDENCY grandet.py)
add_custom_command(OUTPUT grandet_pb2.py COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --python_out=${CMAKE_CURRENT_BINARY_DIR} grandet.proto WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/proto MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/proto/grandet.proto)
add_custom_target(python DEPENDS grandet.py grandet_pb2.py)
