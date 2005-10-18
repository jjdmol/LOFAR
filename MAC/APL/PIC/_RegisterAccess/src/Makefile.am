bin_PROGRAMS 		= RegisterAccess

RegisterAccess_CPPFLAGS= \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

RegisterAccess_SOURCES 		= \
	ARAMain.cc \
	ARAAnswer.cc \
	ARAPhysicalModel.cc \
	ARARegisterAccessTask.cc

NOINSTHDRS = \
        ARAAnswer.h \
	ARAConstants.h \
        ARAPhysicalModel.h \
	ARAPropertyDefines.h \
        ARARegisterAccessTask.h

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS =       $(pkginclude_HEADERS)

include_HEADERS =

BUILT_SOURCES =         \
	RegisterAccess.conf \
	RegisterAccess.log_prop

EXTRA_DIST = $(BUILT_SOURCES) $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA = 

sysconf_DATA= \
	RegisterAccess.conf \
	RegisterAccess.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
