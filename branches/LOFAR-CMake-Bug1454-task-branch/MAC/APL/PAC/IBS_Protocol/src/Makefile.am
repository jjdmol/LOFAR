
AM_CPPFLAGS = -I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
	cp $*.ph $(top_builddir)/include/APL/IBS_Protocol

EXTRA_DIST		= IBS_Protocol.prot 

BUILT_SOURCES	= IBS_Protocol.ph

pkgincludedir	= $(includedir)/APL/IBS_Protocol

pkginclude_HEADERS = $(BUILT_SOURCES)

lib_LTLIBRARIES	= libibs_protocol.la

libibs_protocol_la_SOURCES = Package__Version.cc \
	IBS_Protocol.ph \
	IBS_Protocol.cc \
	Beamlet2SubbandMap.cc \
	Pointing.cc

clean-local:
	rm -f *.ph

bin_PROGRAMS = versionibs_protocol

versionibs_protocol_SOURCES      = versionibs_protocol.cc
versionibs_protocol_LDADD        = libibs_protocol.la
versionibs_protocol_DEPENDENCIES = libibs_protocol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
