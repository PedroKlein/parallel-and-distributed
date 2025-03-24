cmake_minimum_required(VERSION 3.10)
project(OpenMP C)

set(CMAKE_C_STANDARD 99)

find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()

add_executable(test src/test.c)
target_link_libraries(test PUBLIC OpenMP::OpenMP_C)

add_executable(bitonic src/bitonic.c)
target_link_libraries(bitonic PUBLIC OpenMP::OpenMP_C)