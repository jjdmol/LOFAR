lib_LTLIBRARIES	= libvirtualroute.la

libvirtualroute_la_CPPFLAGS = \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once
#  -DUSE_PVSSPORT

libvirtualroute_la_SOURCES = \
	$(DOCHDRS) \
	VirtualRoute.cc \
	WanLSPropertyProxy.cc

NOINSTHDRS = WanLSPropertyProxy.h

INSTHDRS = 

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS	=	$(pkginclude_HEADERS) $(BUILT_SOURCES)

BUILT_SOURCES = 	

EXTRA_DIST =   

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
