

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
	log4cplus.properties \
	ABS_Protocol.ph \
	abs.ns \
	abs.top \
	pac.ns \
	pac.top \
	beamserver.conf

EXTRA_DIST = \
	ABS_Protocol.ph \
	abs.ns \
	abs.top

sysconf_DATA = \
	log4cplus.properties \
	pac.ns \
	pac.top \
	beamserver.conf

include_HEADERS = \
	$(top_builddir)/src/ABS_Protocol.ph

%.properties: %.properties.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

lib_LTLIBRARIES = libabs.la

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
	gnuplot_i.h \
	gnuplot_i.cc \
	$(top_builddir)/src/ABS_Protocol.cc

#	ABSBeamletStats.h \
#	ABSBeamletStats.cc \
#	ABSBeamletStatsPSet.h

bin_PROGRAMS = BeamServer

BeamServer_SOURCES = \
	ABSBeamServerTask.h \
	ABSBeamServerTask.cc

BeamServer_LDADD = \
	$(top_builddir)/src/libabs.la -lrsp

include $(lofar_sharedir)/Makefile.common
