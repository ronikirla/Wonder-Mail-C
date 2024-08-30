# Compiler
CC = gcc

# Compiler flags
CFLAGS = -g -O3 -fopenmp

# Source files
SRCS = $(wildcard src/*.c)

# Output executable
TARGET = opt.exe

# Default target
all: $(TARGET)

# Build the target executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean target (optional)
clean:
	rm -f $(TARGET)
