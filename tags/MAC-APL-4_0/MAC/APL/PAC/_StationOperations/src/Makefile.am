lib_LTLIBRARIES 	= libstationoperations.la

libstationoperations_la_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once
#  -DUSE_PVSSPORT

libstationoperations_la_SOURCES	= \
	StationOperations.cc

BUILT_SOURCES = 

EXTRA_DIST = 

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
