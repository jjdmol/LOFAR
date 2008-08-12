
AM_CPPFLAGS = \
	-I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
	cp $*.ph $(top_builddir)/include/APL/RSP_Protocol

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
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

librsp_protocol_la_SOURCES = Package__Version.cc  \
	$(BUILT_SOURCES) \
	AllRegisterState.cc \
	BeamletWeights.cc \
	BypassSettings.cc \
	HBASettings.cc \
	MEPHeader.cc \
	MEPData.cc \
	RCUSettings.cc \
	RSUSettings.cc \
	SPUStatus.cc \
	Statistics.cc \
	SubbandSelection.cc \
	SystemStatus.cc \
	TBBSettings.cc \
	TDStatus.cc \
	Versions.cc \
	WGSettings.cc \
	XCStatistics.cc

clean-local:
	rm -f *.ph

bin_PROGRAMS = versionrsp_protocol

versionrsp_protocol_SOURCES      = versionrsp_protocol.cc
versionrsp_protocol_LDADD        = librsp_protocol.la
versionrsp_protocol_DEPENDENCIES = librsp_protocol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
