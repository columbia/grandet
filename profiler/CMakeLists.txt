find_package (Boost COMPONENTS program_options regex system timer REQUIRED)
find_package (CURL REQUIRED)
find_library (CURLPP_LIBRARY NAMES curlpp)

include_directories(../actor)

add_executable(profiler profiler.cpp)

target_link_libraries(profiler actor decider common ${LIBAWS_LIBRARY} ${CURL_LIBRARY} ${CURLPP_LIBRARY} ${Boost_LIBRARIES})
