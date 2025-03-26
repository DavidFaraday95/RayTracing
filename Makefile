# Makefile for RayTracing project
# Compiler and flags
CC = gcc
CFLAGS = -I"C:/raylib-5.5/include" -L"C:/raylib-5.5/lib" -lraylib -lgdi32 -lwinmm
TARGET = RayTracing.exe
SOURCE = RayTracing.c

# Default target: build and run
all: build run

# Build the executable
build: $(SOURCE)
	$(CC) -o $(TARGET) $(SOURCE) $(CFLAGS)

# Run the executable
run: $(TARGET)
	./$(TARGET)

# Clean up
clean:
	del $(TARGET)

.PHONY: all build run clean