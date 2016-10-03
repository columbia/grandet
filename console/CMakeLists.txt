find_package (Boost COMPONENTS system)
find_library (READLINE_LIBRARY NAMES readline)

add_executable(console console.cpp)
target_link_libraries(console common pthread proto ${Boost_LIBRARIES} ${READLINE_LIBRARY})
