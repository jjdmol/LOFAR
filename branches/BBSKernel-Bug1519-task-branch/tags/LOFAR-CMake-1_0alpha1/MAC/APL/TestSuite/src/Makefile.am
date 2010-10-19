#pkgincludedir = $(includedir)/Suite

NOINSTHDRS = 

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS	=	$(pkginclude_HEADERS)
      
lib_LTLIBRARIES         = libtestsuite.la

libaplcommon_la_CPPFLAGS= \
	-I$(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
        -fdiagnostics-show-location=once

libtestsuite_la_SOURCES= Package__Version.cc  $(DOCHDRS) \
        suite.cpp \
        test.cpp

BUILT_SOURCES = 

EXTRA_DIST =

clean-local:
	rm -f *.ph

bin_PROGRAMS = versiontestsuite

versiontestsuite_SOURCES      = versiontestsuite.cc
versiontestsuite_LDADD        = libtestsuite.la
versiontestsuite_DEPENDENCIES = libtestsuite.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

