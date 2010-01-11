
AM_CPPFLAGS = -I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
	cp $*.ph $(top_builddir)/include/APL/BS_Protocol

EXTRA_DIST		= BS_Protocol.prot 

BUILT_SOURCES	= BS_Protocol.ph 

pkgincludedir	= $(includedir)/APL/BS_Protocol

pkginclude_HEADERS = $(BUILT_SOURCES)

lib_LTLIBRARIES	= libbs_protocol.la 

libbs_protocol_la_SOURCES = Package__Version.cc \
	BS_Protocol.ph \
	BS_Protocol.cc \
	Beamlet2SubbandMap.cc \
	Pointing.cc

clean-local:
	rm -f *.ph

bin_PROGRAMS = versionbs_protocol

versionbs_protocol_SOURCES      = versionbs_protocol.cc
versionbs_protocol_LDADD        = libbs_protocol.la
versionbs_protocol_DEPENDENCIES = libbs_protocol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
