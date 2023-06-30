# Makefile

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra

# Source directory
SRC_DIR = src

# Build directory for object files
BUILD_DIR = build

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Target executable
TARGET = applyenv

.PHONY: test

test: $(TARGET)
	@make $(TARGET)
	@if [ $$? -eq 0 ]; then \
		INTERPOLATION_TEST_VAR=Interpolated ./$(TARGET) --systemTest; \
	fi

# Default target
all: $(TARGET)

# Rule for building the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Rule for compiling C source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove object files and the target executable
clean:
	rm -f $(OBJS) $(TARGET)
