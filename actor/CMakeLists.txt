add_library (actor redisactor.cpp s3actor.cpp dummyactor.cpp fsactor.cpp)

add_executable(s3actor_test s3actor_test.cpp)
target_link_libraries(s3actor_test actor common ${LIBAWS_LIBRARY} ${GTEST_BOTH_LIBRARIES} pthread)

add_test(s3actor_test s3actor_test)
