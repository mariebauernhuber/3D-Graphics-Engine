CC := g++

# Use pkg-config to get exact Nix store paths for the LSP
SDL_CFLAGS := $(shell pkg-config --cflags sdl3 sdl3-ttf)
SDL_LIBS   := $(shell pkg-config --libs sdl3 sdl3-ttf)

IMGUI_DIR := imgui
IMGUI_BACKENDS := $(IMGUI_DIR)/backends

CPPFLAGS := -Wall -Wextra -g -Iinclude $(SDL_CFLAGS)
CPPFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_BACKENDS)
LDFLAGS  := $(SDL_LIBS) -lm -lGL

SRC_DIR   := src
BUILD_DIR := build

# Your app sources
SRCS      := $(wildcard $(SRC_DIR)/*.cpp)

# ImGui sources (explicit list to avoid unwanted files)
IMGUI_SRCS := $(IMGUI_DIR)/imgui.cpp \
              $(IMGUI_DIR)/imgui_demo.cpp \
              $(IMGUI_DIR)/imgui_draw.cpp \
              $(IMGUI_DIR)/imgui_widgets.cpp \
              $(IMGUI_DIR)/imgui_tables.cpp \
              $(IMGUI_BACKENDS)/imgui_impl_sdl3.cpp \
              $(IMGUI_BACKENDS)/imgui_impl_opengl3.cpp \
	      $(IMGUI_BACKENDS)/imgui_impl_sdlrenderer3.cpp

OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
IMGUI_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(IMGUI_SRCS)))

ALL_OBJS := $(OBJS) $(IMGUI_OBJS)

TARGET := $(BUILD_DIR)/app

all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	$(CC) $(ALL_OBJS) -o $@ $(LDFLAGS)

# Your app object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

# ImGui core files
$(BUILD_DIR)/%.o: $(IMGUI_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

# ImGui backend files
$(BUILD_DIR)/%.o: $(IMGUI_BACKENDS)/%.cpp | $(BUILD_DIR)
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
