lib_LTLIBRARIES	= libarrayoperations.la

libarrayoperations_la_CPPFLAGS = \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once
#  -DUSE_PVSSPORT

libarrayoperations_la_SOURCES = \
	ArrayOperations.cc

BUILT_SOURCES = 	

EXTRA_DIST =   

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
