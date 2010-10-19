BORLAND_PATH = C:\Borland\BCC55
DEFS = -I. -I../lib -I$(BORLAND_PATH)\include -DWIN32 
CC = $(BORLAND_PATH)\Bin\bcc32.exe
LINK = $(BORLAND_PATH)\Bin\ILINK32.EXE
CPPFLAGS = $(DEFS) -w-8027 -w-8026 -w-8057 -w-8022 -tWC -v

OBJECTS =  IterSemanticsTest.obj print_util.obj main.obj \
TestExceptions.obj JoinExample.obj Example.obj

DEPENDENCIES = ../lib/dtl.lib

LDFLAGS =   -L$(BORLAND_PATH)\lib;$(BORLAND_PATH)\lib\PSDK -v

all: tests

clean:
	-@if exist *.obj del *.obj                 >nul 
	-@if exist example.exe del example.exe    >nul


tests: $(OBJECTS) $(DEPENDENCIES)
	$(LINK) $(LDFLAGS) $(OBJECTS), tests, ,..\lib\dtl.lib c0x32.obj CW32.LIB IMPORT32.LIB odbc32.lib

