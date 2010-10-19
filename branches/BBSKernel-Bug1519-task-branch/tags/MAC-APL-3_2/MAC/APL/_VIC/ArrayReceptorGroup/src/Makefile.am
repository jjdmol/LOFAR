lib_LTLIBRARIES = libarrayreceptorgroup.la

libarrayreceptorgroup_la_CPPFLAGS = \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libarrayreceptorgroup_la_SOURCES = \
	ArrayReceptorGroup.cc

BUILT_SOURCES = 	

EXTRA_DIST =   

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
