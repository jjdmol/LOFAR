
AM_CPPFLAGS = \
	-I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
	cp $*.ph $(top_builddir)/include/MD_Protocol

EXTRA_DIST = \
	MD_Protocol.prot 

BUILT_SOURCES = \
	MD_Protocol.ph \
	MD_Protocol.cc

pkgincludedir=$(includedir)/MD_Protocol
pkginclude_HEADERS = \
	$(BUILT_SOURCES)

lib_LTLIBRARIES         = libmd_protocol.la

libmd_protocol_la_SOURCES = Package__Version.cc  \
	$(BUILT_SOURCES) \
	BeamletAllocation.cc \
	BlAllocArr.cc

clean-local:
	rm -f *.ph

bin_PROGRAMS = versionmd_protocol

versionmd_protocol_SOURCES      = versionmd_protocol.cc
versionmd_protocol_LDADD        = libmd_protocol.la
versionmd_protocol_DEPENDENCIES = libmd_protocol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
