
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

%.properties: %.properties.in
	cp $< $@

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

lib_LTLIBRARIES = libabsdevel.la libabs.la

#include_HEADERS =

libabs_ladir = $(prefix)/include

libabs_la_SOURCES = \
	Beam.h \
	Beam.cc \
	Beamlet.h \
	Beamlet.cc

libabsdevel_ladir = $(prefix)/include
libabsdevel_la_HEADERS = \
	Pointing.h \
	Beamlet2SubbandMap.h \
	$(top_builddir)/src/BS_Protocol.ph

libabsdevel_la_SOURCES = \
	Pointing.cc \
	Beamlet2SubbandMap.cc \
	$(top_builddir)/src/BS_Protocol.cc

bin_PROGRAMS = BeamServer

BeamServer_SOURCES = \
	BeamServer.h \
	BeamServer.cc

BeamServer_LDADD = \
	$(top_builddir)/src/libabs.la \
	$(top_builddir)/src/libabsdevel.la 

BeamServer_DEPEND = \
	$(LOFAR_DEPEND)

bin_SCRIPTS = \
	beamserver

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
