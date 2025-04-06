cmake_minimum_required(VERSION 3.10)
project(OpenMP C)

set(CMAKE_C_STANDARD 99)

find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()

add_executable(test src/test.c)
target_link_libraries(test PUBLIC OpenMP::OpenMP_C m)

add_executable(bitonic src/bitonic.c)
target_link_libraries(bitonic PUBLIC OpenMP::OpenMP_C m)

add_executable(bitonic_parallel src/bitonic_parallel.c)
target_link_libraries(bitonic_parallel PUBLIC OpenMP::OpenMP_C)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/output
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/reports
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    DEPENDS ${CMAKE_SOURCE_DIR}/data
    COMMENT "Creating output directory and copying data folder to build directory"
)

add_custom_target(copy_data ALL DEPENDS ${CMAKE_BINARY_DIR}/data)
