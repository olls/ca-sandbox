CC          := clang++

TARGET      := ca-sandbox

SRCDIR      := src
INCDIR      := include
OBJ_DIR     := obj
TARGETDIR   := bin
DOCSDIR     := docs

CFLAGS      := -Werror -Ofast -std=c++14 `sdl2-config --cflags`
LIB         := -lSDL2 -lGLEW -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -lXcursor -lfreetype -I/usr/include/freetype2
INC         := -I$(INCDIR) -I/usr/local/include
INCDEP      := -I$(INCDIR)

SOURCES     := $(shell find $(SRCDIR) -type f -name '*.cpp')

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	BUILD_DIR = debug-build
	CFLAGS += -O0 -g
else
  BUILD_DIR = build
endif

OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst $(SRCDIR)/%,$(OBJ_DIR)/%,$(SOURCES:.cpp=.o)))

all: $(TARGET)

clean:
	@rm -rf $(BUILD_DIR)/$(OBJ_DIR)
	@mkdir $(BUILD_DIR)/$(OBJ_DIR)
	@rm -rf $(BUILD_DIR)/$(TARGETDIR)
	@mkdir $(BUILD_DIR)/$(TARGETDIR)
	@rm -rf $(DOCSDIR)
	@mkdir $(DOCSDIR)

docs:
	./tools/doxygen/build/bin/doxygen Doxyfile

# Pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

# Link

$(TARGET): $(OBJECTS)
	$(CC) -o $(BUILD_DIR)/$(TARGETDIR)/$(TARGET) $^ $(LIB)

# Compile
$(BUILD_DIR)/$(OBJ_DIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(USER_CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(USER_CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.cpp > $(BUILD_DIR)/$(OBJ_DIR)/$*.d
	@cp -f $(BUILD_DIR)/$(OBJ_DIR)/$*.d $(BUILD_DIR)/$(OBJ_DIR)/$*.d.tmp
	@sed -e 's|.*:|$(BUILD_DIR)/$(OBJ_DIR)/$*.o:|' < $(BUILD_DIR)/$(OBJ_DIR)/$*.d.tmp > $(BUILD_DIR)/$(OBJ_DIR)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILD_DIR)/$(OBJ_DIR)/$*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILD_DIR)/$(OBJ_DIR)/$*.d
	@rm -f $(BUILD_DIR)/$(OBJ_DIR)/$*.d.tmp

# Non-File Targets
.PHONY: all remake clean cleaner docs