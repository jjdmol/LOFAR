

AM_CXXFLAGS = \
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
	EPA_Protocol.ph \
	log4cplus.properties \
	abs.ns \
	abs.top

EXTRA_DIST = \
	ABS_Protocol.ph \
	EPA_Protocol.ph \
	log4cplus.properties \
	abs.ns \
	abs.top

include_HEADERS = \
	ABSConstants.h \
	$(top_builddir)/src/ABS_Protocol.ph \
	$(top_builddir)/src/EPA_Protocol.ph

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

noinst_LTLIBRARIES = libabs.la

libabs_la_SOURCES = \
	ABSDirection.h \
	ABSDirection.cc \
	ABSPointing.h \
	ABSPointing.cc \
	ABSSpectralWindow.h \
	ABSSpectralWindow.cc \
	ABSBeam.h \
	ABSBeam.cc \
	ABSBeamlet.h \
	ABSBeamlet.cc \
	ABSBeamletStats.h \
	ABSBeamletStats.cc \
	ABSBeamletStatsPSet.h \
	gnuplot_i.h \
	gnuplot_i.cc \
	$(top_builddir)/src/ABS_Protocol.cc \
	$(top_builddir)/src/EPA_Protocol.cc

bin_PROGRAMS = BeamServer

BeamServer_SOURCES = \
	ABSBeamServerTask.h \
	ABSBeamServerTask.cc

BeamServer_LDADD = \
	$(top_builddir)/src/libabs.la -lgcf -lrsp

include $(lofar_sharedir)/Makefile.common
