CC          := clang++

TARGET      := cellular-automata

SRCDIR      := src
INCDIR      := include
BUILDDIR    := obj
TARGETDIR   := bin

CFLAGS      := -Werror -g -O0 -std=c++14 `sdl2-config --cflags`
LIB         := -lSDL2 -lGLEW -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -lXcursor -lfreetype -I/usr/include/freetype2
INC         := -I$(INCDIR) -I/usr/local/include
INCDEP      := -I$(INCDIR)


SOURCES     := $(shell find $(SRCDIR) -type f -name *.cpp)
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))

all: $(TARGET)

clean:
	@rm -rf $(BUILDDIR)
	@rm -rf $(TARGETDIR)

# Pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

# Link
$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

# Compile
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.cpp > $(BUILDDIR)/$*.d
	@cp -f $(BUILDDIR)/$*.d $(BUILDDIR)/$*.d.tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.o:|' < $(BUILDDIR)/$*.d.tmp > $(BUILDDIR)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.d
	@rm -f $(BUILDDIR)/$*.d.tmp

#Non-File Targets
.PHONY: all remake clean cleaner