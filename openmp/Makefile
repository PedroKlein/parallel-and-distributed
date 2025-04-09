CC = gcc
CFLAGS = -fopenmp -O2 -std=c99
LDFLAGS = -fopenmp -lm

ifdef PROFILING
	CFLAGS = $(CFLAGS) -pg
	LDFLAGS = $(LDFLAGS) -pg
endif

TARGETS = build/test build/bitonic build/bitonic_parallel_v1 build/bitonic_parallel
SRCS = src/test.c src/bitonic.c src/bitonic_parallel_v1.c src/bitonic_parallel.c
OBJS = $(SRCS:src/%.c=build/%.o)

all: $(TARGETS)

build/test: build/test.o
	@mkdir -p build
	$(CC) -o $@ $^ $(LDFLAGS)

build/bitonic: build/bitonic.o
	@mkdir -p build
	$(CC) -o $@ $^ $(LDFLAGS)

build/bitonic_parallel_v1: build/bitonic_parallel_v1.o
	@mkdir -p build
	$(CC) -o $@ $^ $(LDFLAGS)

build/bitonic_parallel: build/bitonic_parallel.o
	@mkdir -p build
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

output:
	@mkdir -p output

run_test: output build/test
	./build/test

run_bitonic: output build/bitonic
	./build/bitonic

run_bitonic_parallel_v1: output build/bitonic_parallel_v1
	OMP_PROC_BIND=TRUE ./build/bitonic_parallel_v1

run_bitonic_parallel: output build/bitonic_parallel
	OMP_PROC_BIND=TRUE ./build/bitonic_parallel
clean:
	rm -rf build gmon.out