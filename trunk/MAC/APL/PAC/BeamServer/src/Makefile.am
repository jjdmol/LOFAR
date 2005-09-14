
#
# if USE_CAL_THREAD is defined in Calibration/src/Makefile.am it should be
# defined here as well and vice versa
#

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-DUSE_CAL_THREAD

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	BS_Protocol.ph \
	BeamServerPorts.conf \
	BeamServer.conf \
	BeamServer.log_prop

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA = \
	BeamServerPorts.conf \
	BeamServer.conf \
	BeamServer.log_prop

include_HEADERS = \
	$(top_builddir)/src/BS_Protocol.ph

%.properties: %.properties.in
	cp $< $@

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

lib_LTLIBRARIES = libabs.la

libabs_ladir = $(prefix)/include
libabs_la_HEADERS = \
	Beamlet2SubbandMap.h \
	Pointing.h

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

clean-local:
	rm -f *.ph

include $(lofar_sharedir)/Makefile.common
