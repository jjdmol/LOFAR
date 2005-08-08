

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-DEPA_Y_POL_IMAG_FIX \
	-DWEIGHTS_IN_NETWORK_ORDER \
	-DUSE_GNUPLOT

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

BUILT_SOURCES = \
	ABS_Protocol.ph \
	ABS_Protocol_v2.ph \
	BeamServerPorts.conf \
	BeamServer.conf

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	BeamServerPorts.conf \
	BeamServer.conf

include_HEADERS = \
	$(top_builddir)/src/ABS_Protocol.ph \
	$(top_builddir)/src/ABS_Protocol_v2.ph

%.properties: %.properties.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

lib_LTLIBRARIES = libabs.la

libabs_la_SOURCES = \
	Beamlet2SubbandMap.h \
	Beamlet2SubbandMap.cc \
	Direction.h \
	Direction.cc \
	Pointing.h \
	Pointing.cc \
	SpectralWindow.h \
	SpectralWindow.cc \
	SpectralWindowConfig.h \
	SpectralWindowConfig.cc \
	Beam.h \
	Beam.cc \
	Beamlet.h \
	Beamlet.cc \
	gnuplot_i.h \
	gnuplot_i.cc \
	$(top_builddir)/src/ABS_Protocol.cc \
	$(top_builddir)/src/ABS_Protocol_v2.cc

bin_PROGRAMS = BeamServer

BeamServer_SOURCES = \
	BeamServer.h \
	BeamServer.cc

BeamServer_LDADD = \
	$(top_builddir)/src/libabs.la -lrsp

#include $(lofar_sharedir)/Makefile.common
include ../Makefile.common
