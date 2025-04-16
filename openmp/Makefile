CC = gcc
DEFAULT_CFLAGS = -fopenmp -O2 -std=c99
DEFAULT_LDFLAGS = -fopenmp -lm

ifdef PROFILING
	CFLAGS = $(DEFAULT_CFLAGS) -pg
	LDFLAGS = $(DEFAULT_LDFLAGS) -pg
else
	CFLAGS = $(DEFAULT_CFLAGS)
	LDFLAGS = $(DEFAULT_LDFLAGS)
endif

TARGETS = build/test build/sort
SRCS = src/test.c src/sort.c
OBJS = $(SRCS:src/%.c=build/%.o)

# VTune parameters
BINARY = ./build/sort
INPUT_FILE = in_16777216.in
TASK_THRESHOLD = 1024
SORT_METHOD = bitonic_parallel
OMP_THREADS = 1
ANALYSIS_TYPE = hpc-performance
RESULT_DIR = vtune_results_$(ANALYSIS_TYPE)

all: $(TARGETS)

build/test: build/test.o
	@mkdir -p build
	$(CC) -o $@ $^ $(LDFLAGS)

build/sort: build/sort.o
	@mkdir -p build
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

output:
	@mkdir -p output

run_test: output build/test
	./build/test

run_sort: output build/sort
	./build/sort

clean:
	rm -rf build gmon.out vtune_*

alloc-hype:
	salloc -p hype -J bitonic_sort -t 01:00:00

run_vtune: $(BINARY)
	@mkdir -p $(RESULT_DIR)
	OMP_NUM_THREADS=$(OMP_THREADS) OMP_PROC_BIND=TRUE vtune -collect $(ANALYSIS_TYPE) \
		-result-dir $(RESULT_DIR) -- $(BINARY) -i $(INPUT_FILE) -t $(TASK_THRESHOLD) \
		-csv -sort $(SORT_METHOD)
	vtune -r $(RESULT_DIR) -format=csv -report summary