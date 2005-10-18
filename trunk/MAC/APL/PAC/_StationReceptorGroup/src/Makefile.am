pkgincludedir = $(includedir)/StationReceptorGroup

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS =       $(pkginclude_HEADERS)

lib_LTLIBRARIES 	= libstationreceptorgroup.la

libstationreceptorgroup_la_CPPFLAGS= \
	-I$(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libstationreceptorgroup_la_SOURCES	= \
	StationReceptorGroup.cc

BUILT_SOURCES = 

EXTRA_DIST = 

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
