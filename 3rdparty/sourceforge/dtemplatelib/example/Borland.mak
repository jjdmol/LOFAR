BORLAND_PATH = C:\Borland\BCC55
DEFS = -I. -I../lib -I$(BORLAND_PATH)\include -DWIN32 
CC = $(BORLAND_PATH)\Bin\bcc32.exe
LINK = $(BORLAND_PATH)\Bin\ILINK32.EXE

CPPFLAGS = $(DEFS) -w-8027 -w-8026 -w-8057 -w-8022 -tWC -v
OBJECTS =  SpecialQryExample.obj ShareConn.obj StoredProc.obj \
range.obj ReadDataNoMatches.obj example_core.obj WriteData.obj \
SimpleDynamicRead.obj ReadJoinedData.obj ReadData.obj  \
DynamicIndexedViewExample.obj example.obj main.obj IndexedViewExample.obj CStringExample.obj

DEPENDENCIES = ../lib/dtl.lib

LDFLAGS =   -L$(BORLAND_PATH)\lib;$(BORLAND_PATH)\lib\PSDK -v

all: example

clean:
	-@if exist *.obj del *.obj                 >nul 
	-@if exist example.exe del example.exe    >nul


example: $(OBJECTS) $(DEPENDENCIES)
	$(LINK) $(LDFLAGS) $(OBJECTS), example, , ..\lib\dtl.lib c0x32.obj CW32.LIB IMPORT32.LIB odbc32.lib
