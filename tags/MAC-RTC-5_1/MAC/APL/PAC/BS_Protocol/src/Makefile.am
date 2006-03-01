
AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $@ $(top_builddir)/include/APL/BS_Protocol

BUILT_SOURCES = \
	BS_Protocol.ph

EXTRA_DIST =

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
