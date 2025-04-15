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
	rm -rf build gmon.out