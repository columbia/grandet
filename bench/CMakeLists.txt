find_package (Boost COMPONENTS system)
add_executable(resbench resbench.cpp)
add_executable(latbench latbench.cpp)
target_link_libraries(resbench pthread grandet ${Boost_LIBRARIES})
target_link_libraries(latbench pthread grandet ${LIBAWS_LIBRARY} ${Boost_LIBRARIES})
include_directories(../sdk/cpp)
