

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
	BS_Protocol.ph \
	BeamServerPorts.conf \
	BeamServer.conf

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	BeamServerPorts.conf \
	BeamServer.conf

include_HEADERS = \
	$(top_builddir)/src/BS_Protocol.ph

%.properties: %.properties.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

lib_LTLIBRARIES = libabs.la

libabs_la_SOURCES = \
	Beamlet2SubbandMap.h \
	Beamlet2SubbandMap.cc \
	Pointing.h \
	Pointing.cc \
	Beam.h \
	Beam.cc \
	Beamlet.h \
	Beamlet.cc \
	gnuplot_i.h \
	gnuplot_i.cc \
	$(top_builddir)/src/BS_Protocol.cc

bin_PROGRAMS = BeamServer

BeamServer_SOURCES = \
	BeamServer.h \
	BeamServer.cc

BeamServer_LDADD = \
	$(top_builddir)/src/libabs.la -lcal -lcaldevel -lrsp

#include $(lofar_sharedir)/Makefile.common
include ../Makefile.common
