
AM_CPPFLAGS = \
	-I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/APL/TBB_Protocol

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/APL/TBB_Protocol

EXTRA_DIST = \
	TBB_Protocol.prot

BUILT_SOURCES = \
	TBB_Protocol.ph\
	TBB_Protocol.cc
	
pkgincludedir=$(includedir)/APL/TBB_Protocol
pkginclude_HEADERS = \
	$(BUILT_SOURCES)

lib_LTLIBRARIES         = libtbb_protocol.la

libtbb_protocol_la_SOURCES = \
	$(BUILT_SOURCES) 

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
