
AM_CPPFLAGS = \
	-I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/APL/RSP_Protocol

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/APL/RSP_Protocol

EXTRA_DIST = \
	EPA_Protocol.prot \
	RSP_Protocol.prot \
	RSPFE_Protocol.prot

BUILT_SOURCES = \
	EPA_Protocol.ph \
	EPA_Protocol.cc \
	RSP_Protocol.ph \
	RSP_Protocol.cc \
	RSPFE_Protocol.ph \
	RSPFE_Protocol.cc

pkgincludedir=$(includedir)/APL/RSP_Protocol
pkginclude_HEADERS = \
	$(BUILT_SOURCES)

lib_LTLIBRARIES         = librsp_protocol.la

librsp_protocol_la_SOURCES = \
	$(BUILT_SOURCES) \
	BeamletWeights.cc \
	Clocks.cc \
	RCUSettings.cc \
	RSUSettings.cc \
	Statistics.cc \
	SubbandSelection.cc \
	SystemStatus.cc \
	Versions.cc \
	WGSettings.cc \
	XCStatistics.cc \
	MEPHeader.cc \
	MEPData.cc

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
