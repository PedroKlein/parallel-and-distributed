# Makefile for MPI Matrix Multiplication

all: #mpi_coletiva mpi_p2p_bloqueante mpi_p2p_naobloqueante
	mkdir -p build
	mpicc mpi_coletiva.c -o build/mpi_coletiva
	mpicc mpi_p2p_bloqueante.c -o build/mpi_p2p_bloqueante
	mpicc mpi_p2p_naobloqueante.c -o build/mpi_p2p_naobloqueante

clean:
	rm build