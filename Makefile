# Supply make with DEBUG=1 to switch to debug build
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	BUILD_DIR = debug-build
	DEBUG_FLAGS = -O0 -ggdb -D_DEBUG
else
	DEBUG_FLAGS =
  BUILD_DIR = build
endif

CC          := clang++

TARGET      := ca-sandbox

SRCDIR      := src
INC         := -Iinclude -Iinclude/imgui
OBJ_DIR     := obj
DYNAMIC_LIBS_DIR := dynamic
TARGETDIR   := bin
DOCSDIR     := docs

SO_PATH     := $(BUILD_DIR)/$(DYNAMIC_LIBS_DIR)
LINKER_FLAGS := -Wl,-export-dynamic,--no-undefined,-rpath,$(SO_PATH)

CFLAGS      := -Werror -Ofast -std=c++14 `sdl2-config --cflags`

# Compiled using a dynamic library and a loader program.
# See here [http://amir.rachum.com/blog/2016/09/17/shared-libraries/] for a good explanation of dynamic libraries.

LIB_LIBS    :=
LOADER_LIBS := `sdl2-config --libs` -lGLEW -lGL -lGLU -ldl

LIB_SRCDIRS = $(SRCDIR)/ca-sandbox $(SRCDIR)/imgui $(SRCDIR)/interface
LIB_SOURCES = $(shell find $(LIB_SRCDIRS) -type f -name '*.cpp')
LIB_OBJECTS = $(LIB_SOURCES:$(SRCDIR)/%.cpp=$(BUILD_DIR)/$(OBJ_DIR)/%.o)

# Loader sources are everything in src/*.cpp and engine/**/*.cpp
LOADER_SOURCES = $(shell find $(SRCDIR) -maxdepth 1 -type f -name '*.cpp') $(shell find $(SRCDIR)/engine -type f -name '*.cpp')
LOADER_OBJECTS = $(LOADER_SOURCES:$(SRCDIR)/%.cpp=$(BUILD_DIR)/$(OBJ_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(BUILD_DIR)/$(TARGETDIR)/$(TARGET)

# Link loader objects into executable
$(BUILD_DIR)/$(TARGETDIR)/$(TARGET): $(SO_PATH)/libca-sandbox.so $(LOADER_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_FLAGS) -o $@ $(filter-out $<,$^) $(LOADER_LIBS) $(LINKER_FLAGS)

# Link library objects into shared library
$(SO_PATH)/libca-sandbox.so: $(LIB_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_FLAGS) $(LIB_LIBS) -shared -o $@ $^

# Compile all objects
$(BUILD_DIR)/$(OBJ_DIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -fPIC $(USER_CFLAGS) $(INC) -MMD -c $< -o $@

-include $(LOADER_OBJECTS:.o=.d) $(LIB_OBJECTS:.o=.d)


clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DOCSDIR)

clean-ca-sandbox:
	rm -rf $(BUILD_DIR)/$(OBJ_DIR)/ca-sandbox

clean-engine:
	rm -rf $(BUILD_DIR)/$(OBJ_DIR)/engine

clean-imgui:
	rm -rf $(BUILD_DIR)/$(OBJ_DIR)/imgui

docs:
	@mkdir -p $(DOCSDIR)
	./tools/doxygen/build/bin/doxygen Doxyfile


# Non-File Targets
.PHONY: all link clean cleaner docs