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

libsuite_la_SOURCES= Package__Version.cc  $(DOCHDRS) \
        suite.cpp \
        test.cpp

BUILT_SOURCES = 

EXTRA_DIST =

clean-local:
	rm -f *.ph

bin_PROGRAMS = versionsuite

versionsuite_SOURCES      = versionsuite.cc
versionsuite_LDADD        = libsuite.la
versionsuite_DEPENDENCIES = libsuite.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

