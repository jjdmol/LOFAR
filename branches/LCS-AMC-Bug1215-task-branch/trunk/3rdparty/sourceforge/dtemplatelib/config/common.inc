# Written for GNU -*- makefile -*-.

SH = bash # bourne-again shell

CXX = $(CC)
CXXFLAGS = $(CFLAGS)
CPPFLAGS = $(PROJECT_CPPFLAGS)

# Someone goofed
ifndef TARGET
$(error no TARGET)
endif

# Default to debug builds
ifndef BUILD
BUILD = debug
endif

# Place targets into build-specific directories
TARGET := $(BUILD)/$(TARGET)

ifneq (,$(findstring release,$(BUILD)))
CFLAGS += -O
CPPFLAGS += -DNDEBUG
else
ifneq (,$(findstring debug,$(BUILD)))
CFLAGS += -g
CPPFLAGS += -D_DEBUG
else
ifneq (,$(findstring profile,$(BUILD)))
else
$(error unknown BUILD: $(BUILD))
endif
endif
endif

#UNICODE configuration flag
#if the build contains the word 'unicode', set the unicode flags
ifneq (,$(findstring unicode,$(BUILD)))
CFLAGS += -D "_UNICODE" -D "UNICODE"
endif

#if the build contains the word 'stlport', use the STLport libraries
#here we assume stlport is located in c:\stlport
ifneq (,$(findstring stlport,$(BUILD)))
CFLAGS += -I "/cygdrive/c/stlport" -I "/cygdrive/c/stlport/old_hp"
endif



# This needs to be early for immediate variable changes
ifneq (,$(findstring msvc,$(PLATFORM)))
EXE := .exe
OBJ := .obj
OUTPUT_OPTION = -Fo$@
else
EXE :=
OBJ := .o
endif

# The default links with CC, which doesn't include needed C++ libraries.
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

# Do not change the extension of source files.  XXX
OBJECTS = $(SOURCES:%.cpp=$(BUILD)/%$(OBJ))

# Not the default with historical make, but definitely what we want
.DELETE_ON_ERROR:
# Never remove intermediary files
.SECONDARY:

# This must ALWAYS be the first target
all: $(BUILD) $(TARGET)
	@echo Built $(TARGET) for $(PLATFORM)

$(BUILD):
	mkdir $(BUILD)

MAKEFILEDEPS += ../config/common.mak


# Default build rule - do not use dependencies
$(BUILD)/%.o: %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

# Default AR rule
%.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)


# Magic for auto-dependencies.  See
# <http://make.paulandlesley.org/autodep.html>
d_file = $(BUILD)/$(*F).d
P_file = $(BUILD)/$(*F).P

# Change default build rule to build in situ and to use header
# dependencies.  These are generic rules; platform makefiles will
# often override them.
# $(BUILD)/%.o: %.cpp
#	@$(CPP) $(CPPFLAGS) $< | sed -n \
#	's,^# *[0-9][0-9]* *"\([^"]*\)".*,\1,p' | sort -u | sed -n \
# 'H ; $$ { \
#  x ; \
#  s/\n/ /g ; \
#  s/^ \(.*\)$$/\1:/ ; \
#  h ; \
#  s,^\(.*\):$$,$@: $(MAKEFILEDEPS) \1,p ; \
#  x ; \
#  p \
# }' >$(P_file)
#	$(COMPILE.cpp) $(OUTPUT_OPTION) $<
#
# -include $(SOURCES:%.cpp=$(BUILD)/%.P)

# Don't confuse make if someone makes a file named "clean"
.PHONY: clean
clean::
	-$(RM) -r $(BUILD)
