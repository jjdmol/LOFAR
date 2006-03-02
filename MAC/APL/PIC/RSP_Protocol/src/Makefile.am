
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

pkgincludedir=$(includedir)/APL/RSP_Protocol
pkginclude_HEADERS = \
	EPA_Protocol.ph \
	RSP_Protocol.ph \
	RSPFE_Protocol.ph

BUILT_SOURCES = \
	EPA_Protocol.cc \
	RSP_Protocol.cc \
	RSPFE_Protocol.cc

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = \
	EPA_Protocol.prot \
	RSP_Protocol.prot \
	RSPFE_Protocol.prot

lib_LTLIBRARIES         = librsp_protocol.la

nodist_librsp_protocol_la_SOURCES = \
	EPA_Protocol.ph \
	RSP_Protocol.ph \
	RSPFE_Protocol.ph \
	EPA_Protocol.cc \
	RSP_Protocol.cc \
	RSPFE_Protocol.cc

librsp_protocol_la_SOURCES = \
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
	rm -f $(BUILT_SOURCES)

include $(top_srcdir)/Makefile.common
