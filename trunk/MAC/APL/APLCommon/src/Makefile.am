AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

lib_LTLIBRARIES = libaplcommon.la

libaplcommon_la_CPPFLAGS =  -DBOOST_DISABLE_THREADS \
							-Wno-deprecated \
							-fmessage-length=0 \
							-fdiagnostics-show-location=once

libaplcommon_la_SOURCES =   $(DOCHDRS) \
							StartDaemon_Protocol.cc \
							Controller_Protocol.cc \
							APLUtilities.cc \
							ControllerDefines.cc \
							PropertySetAnswer.cc \
							CTState.cc \
							ChildControl.cc \
							ParentControl.cc \
							ResourceAllocator.cc 


NOINSTHDRS 				=

INSTHDRS 				= 

pkginclude_HEADERS 		= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS 				= $(pkginclude_HEADERS) $(BUILT_SOURCES)

BUILT_SOURCES 			= $(ph_HEADERS)

EXTRA_DIST 				= 

phdir					=$(includedir)/APL/APLCommon

ph_HEADERS 				= StartDaemon_Protocol.ph \
						  Controller_Protocol.ph

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common

