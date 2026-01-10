CC := g++

# Use pkg-config to get exact Nix store paths for the LSP
SDL_CFLAGS := $(shell pkg-config --cflags sdl3 sdl3-ttf)
SDL_LIBS   := $(shell pkg-config --libs sdl3 sdl3-ttf)

CPPFLAGS := -Wall -Wextra -g -Iinclude $(SDL_CFLAGS)
LDFLAGS  := $(SDL_LIBS) -lm

SRC_DIR   := src
BUILD_DIR := build
SRCS      := $(wildcard $(SRC_DIR)/*.cpp)
OBJS      := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
TARGET    := $(BUILD_DIR)/app

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Fixed: Now uses $(CPPFLAGS) so bear captures the include paths
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

force:
	$(MAKE) clean all

.PHONY: all clean run force
