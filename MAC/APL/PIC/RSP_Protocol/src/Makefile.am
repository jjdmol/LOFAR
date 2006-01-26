
AM_CPPFLAGS = -I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $@ $(top_builddir)/include/APL/RSP_Protocol

BUILT_SOURCES = \
	EPA_Protocol.ph \
	RSP_Protocol.ph \
	RSPFE_Protocol.ph

EXTRA_DIST =

pkgincludedir=$(includedir)/APL/RSP_Protocol
pkginclude_HEADERS = \
	$(BUILT_SOURCES)

lib_LTLIBRARIES         = librsp_protocol.la

librsp_protocol_la_SOURCES = \
	$(BUILT_SOURCES) \
	EPA_Protocol.cc \
	RSP_Protocol.cc \
	RSPFE_Protocol.cc \
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
