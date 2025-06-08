cmake_minimum_required(VERSION 3.10)
project(OpenMP C)

set(CMAKE_C_STANDARD 99)

find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()

find_program(GPROF_FOUND gprof)
if(GPROF_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()

# Add -O2 optimization flag
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

add_executable(test src/test.c)
target_link_libraries(test PUBLIC OpenMP::OpenMP_C m)

add_executable(sort src/sort.c)
target_link_libraries(sort PUBLIC OpenMP::OpenMP_C m)
set_property(TARGET sort PROPERTY ENVIRONMENT "OMP_PROC_BIND=TRUE")

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
