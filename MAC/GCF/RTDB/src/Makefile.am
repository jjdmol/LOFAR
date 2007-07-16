AM_CPPFLAGS			= -I$(top_builddir)/include
AUTOGEN 			= autogen
SUFFIXES 			= .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/GCF/RTDB/


clean-local:
	rm -f *.ph

DOCHDRS				= RTDB_Property.h \
					  RTDB_PropertySet.h \
					  DP_Answer.h  \
					  PropResponse.h
   
librtdb_la_CPPFLAGS	= -I$(top_srcdir)/PVSS/src

BUILT_SOURCES 		= DP_Protocol.ph \
					  DP_Protocol.cc

EXTRA_DIST 			= DP_Protocol.prot \
					  $(sysconf_DATA)

lib_LTLIBRARIES 	= librtdb.la

librtdb_la_SOURCES	= $(BUILT_SOURCES) \
					  RTDB_Property.cc \
					  RTDB_PropertySet.cc \
					  DP_Answer.cc \
					  PropResponse.cc

include $(top_srcdir)/Makefile.common

