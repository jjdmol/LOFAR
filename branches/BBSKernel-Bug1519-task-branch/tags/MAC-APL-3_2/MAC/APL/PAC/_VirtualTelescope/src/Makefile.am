lib_LTLIBRARIES 	= libvirtualtelescope.la

libvirtualtelescope_la_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libvirtualtelescope_la_SOURCES	= \
	VirtualTelescope.cc

BUILT_SOURCES = 

EXTRA_DIST = 

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
