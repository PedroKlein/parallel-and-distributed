cmake_minimum_required(VERSION 3.10)
project(mpi_matmult C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(MPI REQUIRED)

include_directories(include)

set(SOURCES
    src/main.c
    src/collective.c
    src/sync.c
    src/async.c
    src/async_naive.c
)

add_executable(mpi_matmult ${SOURCES})

target_link_libraries(mpi_matmult PRIVATE MPI::MPI_C)