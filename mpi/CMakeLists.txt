cmake_minimum_required(VERSION 3.10)
project(mpi_matrix_multiplication C)

find_package(MPI REQUIRED)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MPI_C_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_C_LINK_FLAGS}")

include_directories(${MPI_C_INCLUDE_PATH})

add_executable(mpi_coletiva mpi_coletiva.c)
add_executable(mpi_p2p_bloqueante mpi_p2p_bloqueante.c)
add_executable(mpi_p2p_naobloqueante mpi_p2p_naobloqueante.c)

target_link_libraries(mpi_coletiva ${MPI_C_LIBRARIES})
target_link_libraries(mpi_p2p_bloqueante ${MPI_C_LIBRARIES})
target_link_libraries(mpi_p2p_naobloqueante ${MPI_C_LIBRARIES})