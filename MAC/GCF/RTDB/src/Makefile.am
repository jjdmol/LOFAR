AM_CPPFLAGS			= -I$(top_builddir)/include
AUTOGEN 			= autogen
SUFFIXES 			= .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
	cp $*.ph $(top_builddir)/include/GCF/RTDB/

clean-local:
	rm -f *.ph

DOCHDRS				= RTDB_PropertySet.h \
					  PropSetResponse.h \
					  DPanswer.h
   
libgcfrtdb_la_CPPFLAGS	= -I$(top_srcdir)/PVSS/src

BUILT_SOURCES 		= DP_Protocol.ph \
					  DP_Protocol.cc

EXTRA_DIST 			= DP_Protocol.prot \
					  $(sysconf_DATA)

pkgincludedir		= $(includedir)/GCF/RTDB
pkginclude_HEADERS	= $(BUILT_SOURCES)

lib_LTLIBRARIES 	= libgcfrtdb.la

libgcfrtdb_la_SOURCES	= $(BUILT_SOURCES) \
					  RTDB_PropertySet.cc \
					  DPservice.cc \
					  DPanswer.cc

include $(top_srcdir)/Makefile.common

