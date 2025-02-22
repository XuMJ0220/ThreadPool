add_executable(test2 test2.cpp ${CMAKE_SOURCE_DIR}/src/threadpool.cpp)
add_executable(test1 test1.cpp ${CMAKE_SOURCE_DIR}/src/threadpool.cpp)
target_link_libraries(test2 pthread)
target_link_libraries(test1 pthread)