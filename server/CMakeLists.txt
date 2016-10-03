find_package (Boost COMPONENTS program_options REQUIRED)

include_directories (../actor ../decider ../estimator)

add_executable(server controller.cpp main.cpp connmgr.cpp memory.cpp lockmgr.cpp accountant.cpp statistic.cpp unixconnmgr.cpp)

target_link_libraries(server decider estimator actor proto common pthread ${LIBAWS_LIBRARY} ${REDIS3M_STATIC_LIBRARIES} ${Boost_LIBRARIES})
