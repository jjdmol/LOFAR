# Written for GNU -*- makefile -*-.
# To invoke build from the compiler you will need to set the registry so that cygwin is in your MSVC path
# To do this you will need to set the following registry key:

# [HKEY_USERS\S-1-5-21-1275210071-1682526488-1343024091-500\Software\Microsoft\DevStudio\6.0\Build System\Components\Platforms\Win32 (x86)\Directories]
# "Path Dirs"="C:\\cygwin\\bin;(rest as before...)"

CC = cl.exe -nologo
CFLAGS += -G6 -GR -GX -W1 -Zm600
CPPFLAGS += -DWIN32 -D_MBCS
LDLIBS += -lodbc32 -lodbccp32
LD = link.exe -nologo -pdb:none
LDFLAGS = -subsystem:console -incremental:no
LDLIBS += -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 \
	-lshell32 -lole32 -loleaut32 -luuid
AR = lib.exe -nologo
ARFLAGS =

ifneq (,$(findstring release,$(BUILD)))
CFLAGS += -O2 -Ob2 -MT
LDFLAGS += -nodefaultlib:libc
LDLIBS += -llibcmt
else
ifneq (,$(findstring debug,$(BUILD)))
CFLAGS := $(subst -g,,$(CFLAGS)) -GZ -Od -Zi -MTd # no -g with cl.exe
LDFLAGS += -debug -nodefaultlib:libcd -map -FIXED:NO 
LDLIBS += -llibcmtd
else
$(error unknown BUILD: $(BUILD))
endif
endif

ifneq (,$(findstring boundschecker,$(TARGET)))
CC := nmcl.exe
LD := nmlink.exe
LDFLAGS += -L"$(shell cygpath -m "$$(dirname "$$(which $(CC))")")"
LDLIBS += bcinterf.lib
endif

SUFFIXES := $(subst .o,$(OBJ),$(SUFFIXES))
SUFFIXES := $(subst .a,.lib,$(SUFFIXES))
.LIBPATTERNS := %.lib

# How to make libraries:
LINK.obj = $(LD) $(LDFLAGS:-L%=-libpath:%) $(TARGET_ARCH)

%.lib: $(OBJECTS)
	$(AR) $(ARFLAGS) -out:$@ $(OBJECTS)


%: %.obj
	$(LINK.obj) $(^:-l%=%.lib) $(LOADLIBES:-l%=%.lib) $(LDLIBS:-l%=%.lib) -out:$@$(EXE)

# see ../config/post.inc
ifdef mydirs
vpath %.lib $(mydirs:%=%/$(BUILD))
endif

# How to make header dependencies:
# See <http://make.paulandlesley.org> and common.inc

# Ignore any standard library headers via grep -v '$(standard_library_include)' 
# Note that for VC .NET this will need to be changed
# to a string that matches the standard library include directory for .NET
standard_library_include = VC98

# Run the MSVC C++ pre-processor (denoted by the cl.exe -E flag)
# to pre-process the source code and extract dependencies.
$(BUILD)/%$(OBJ): %.cpp
	@$(CPP) $(CPPFLAGS) $< 2>/dev/null | grep -v '$(standard_library_include)' | sed -n \
'/^#line / { \
  s,\\\\,/,g ; \
  s,[^"]*"\([^"]*\).*,\1,gp \
}' | sort -u | sed -n \
'H ; $$ { \
  x ; \
  s/\n/ /g ; \
  s,^,$@: $(MAKEFILEDEPS), ; \
  p \
}' >$(P_file) 
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

-include $(SOURCES:%.cpp=$(BUILD)/%.P)


	
