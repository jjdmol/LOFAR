AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

lib_LTLIBRARIES = libaplcommon.la

libaplcommon_la_CPPFLAGS = \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once
#  -DUSE_PVSSPORT

libaplcommon_la_SOURCES = \
	$(DOCHDRS) \
	APLUtilities.cc \
	LogicalDevice.cc \
	StartDaemon.cc \
	PropertySetAnswer.cc \
	ResourceAllocator.cc \
	LogicalDevice_Protocol.cc \
	StartDaemon_Protocol.cc

NOINSTHDRS =

INSTHDRS = 

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS = $(pkginclude_HEADERS) $(BUILT_SOURCES)

BUILT_SOURCES = \
	$(ph_HEADERS)

EXTRA_DIST = 

phdir=$(includedir)/APL/APLCommon
ph_HEADERS = \
	LogicalDevice_Protocol.ph \
	StartDaemon_Protocol.ph

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common

