CC = gcc
CFLAGS = -fopenmp -pg -O2
LDFLAGS = -pg -lm

TARGET = build/bitonic
SRCS = src/bitonic.c
OBJS = $(SRCS:src/%.c=build/%.o)

INPUT ?= simple.in

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p build
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build gmon.out

profile: all
	@mkdir -p report
	@mkdir -p output
	./$(TARGET) -i $(INPUT)
	gprof $(TARGET) gmon.out > report/$(INPUT).report