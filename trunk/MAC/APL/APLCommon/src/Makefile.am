AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

lib_LTLIBRARIES = libaplcommon.la

libaplcommon_la_CPPFLAGS = \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libaplcommon_la_SOURCES = \
	APLUtilities.cc \
	LogicalDevice.cc \
	StartDaemon.cc \
	PropertySetAnswer.cc \
	LogicalDevice_Protocol.cc \
	StartDaemon_Protocol.cc

BUILT_SOURCES = \
	LogicalDevice_Protocol.ph \
	StartDaemon_Protocol.ph

EXTRA_DIST = $(BUILT_SOURCES)

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common

