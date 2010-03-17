
AM_CPPFLAGS = \
	-I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
	cp $*.ph $(top_builddir)/include/APL/ICAL_Protocol

EXTRA_DIST = \
	ICAL_Protocol.prot

BUILT_SOURCES = \
	ICAL_Protocol.ph

pkgincludedir=$(includedir)/APL/ICAL_Protocol
pkginclude_HEADERS = \
	$(BUILT_SOURCES)

lib_LTLIBRARIES = \
	libcal_protocol.la

libcal_protocol_la_SOURCES = Package__Version.cc \
	$(BUILT_SOURCES) \
	ICAL_Protocol.cc \
	AntennaArray.cc \
	AntennaArrayData.cc \
	AntennaGains.cc \
	SpectralWindow.cc \
	SubArray.cc \
	ACC.cc

clean-local:
	rm -f *.ph

bin_PROGRAMS = versioncal_protocol

versioncal_protocol_SOURCES      = versioncal_protocol.cc
versioncal_protocol_LDADD        = libcal_protocol.la
versioncal_protocol_DEPENDENCIES = libcal_protocol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
