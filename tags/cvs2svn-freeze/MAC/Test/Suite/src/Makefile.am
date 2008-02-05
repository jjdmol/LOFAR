pkgincludedir = $(includedir)/Suite

NOINSTHDRS = 

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS	=	$(pkginclude_HEADERS)
      
lib_LTLIBRARIES         = libsuite.la

libaplcommon_la_CPPFLAGS= \
	-I$(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
        -fdiagnostics-show-location=once

libsuite_la_SOURCES= $(DOCHDRS) \
        suite.cpp \
        test.cpp

BUILT_SOURCES = 

EXTRA_DIST =

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common

