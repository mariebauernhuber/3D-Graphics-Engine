# Compiler
CC := g++

# Flags
CFLAGS := -Wall -Wextra -g
LDFLAGS := -lSDL3 -lm

# Directories
SRC_DIR := src
BUILD_DIR := build

# Files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Output executable
TARGET := $(BUILD_DIR)/app

# Default target
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile .c files into .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run

#force recompilation
.PHONY: force
force:
	$(MAKE) clean all
