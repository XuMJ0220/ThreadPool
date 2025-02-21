add_executable(test test1.cpp ${CMAKE_SOURCE_DIR}/src/threadpool.cpp)

target_link_libraries(test pthread)