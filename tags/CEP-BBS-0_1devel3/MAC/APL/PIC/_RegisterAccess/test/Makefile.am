bin_PROGRAMS 		= aratest

TESTS			= #aratest

aratest_CPPFLAGS= \
	-I$(top_srcdir)/../GCF/GCFCommon/src \
	-I$(top_srcdir)/PIC/RegisterAccess/src \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

aratest_SOURCES 		= \
	ARATestAnswer.cc \
	ARATestTask.cc \
	ARATestDriverTask.cc \
	ARATestMain.cc

NOINSTHDRS = \
        ARATestAnswer.h \
        ARATestTask.h \
        ARATestDriverTask.h

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS =       $(pkginclude_HEADERS)

include_HEADERS =

BUILT_SOURCES =         \
	aratest.conf \
	aratest.log_prop

EXTRA_DIST = $(BUILT_SOURCES) $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA = 

sysconf_DATA= \
	aratest.conf \
	aratest.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
