BORLAND_PATH = C:\Borland\BCC55
DEFS = -I. -I$(BORLAND_PATH)\include -DWIN32 
CC = $(BORLAND_PATH)\Bin\bcc32.exe

CPPFLAGS = $(DEFS) -v
OBJECTS =  variant_row.obj validate.obj \
dtl_base_types.obj clib_fwd.obj VariantException.obj RootException.obj \
DBException.obj string_util.obj date_util.obj bind_basics.obj DB_Base.obj \
DBStmt.obj DBConnection.obj CountedPtr.obj BoundIO.obj LocalBCA.obj DBView.obj

all: dtl

clean:
	-@if exist *.obj del *.obj                 >nul
	-del dtl.lib
	

dtl: $(OBJECTS)
	-del dtl.lib
       $(BORLAND_PATH)\Bin\tlib dtl.lib /a $(OBJECTS)
