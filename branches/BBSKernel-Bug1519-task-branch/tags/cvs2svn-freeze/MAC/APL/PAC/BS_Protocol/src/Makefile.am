
AM_CPPFLAGS = \
	-I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/APL/BS_Protocol

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/APL/BS_Protocol

EXTRA_DIST = \
	BS_Protocol.prot

BUILT_SOURCES = \
	BS_Protocol.ph

pkgincludedir=$(includedir)/APL/BS_Protocol
pkginclude_HEADERS = \
	$(BUILT_SOURCES)

lib_LTLIBRARIES = \
	libbs_protocol.la

libbs_protocol_la_SOURCES = \
	$(BUILT_SOURCES) \
	BS_Protocol.cc \
	Beamlet2SubbandMap.cc \
	Pointing.cc

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
