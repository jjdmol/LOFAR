
AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	CAL_Protocol.ph

EXTRA_DIST = 

pkgincludedir=$(includedir)/APL/CAL_Protocol
pkginclude_HEADERS = \
	$(BUILT_SOURCES)

lib_LTLIBRARIES         = libcal_protocol.la

libcal_protocol_la_SOURCES = \
	$(BUILT_SOURCES) \
	CAL_Protocol.cc \
	AntennaArray.cc \
	AntennaArrayData.cc \
	AntennaGains.cc \
	SpectralWindow.cc \
	SubArray.cc \
	ACC.cc

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
