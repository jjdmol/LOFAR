
AM_CXXFLAGS = -I$(top_srcdir)/src

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../GCF/GCFCommon/autogen $<

BUILT_SOURCES = \
	$(top_builddir)/src/ABS_Protocol.ph \
	$(top_builddir)/src/EPA_Protocol.ph \
	log4cplus.properties \
	abs.ns \
	abs.top

EXTRA_DIST = \
	$(top_builddir)/src/ABS_Protocol.ph \
	$(top_builddir)/src/EPA_Protocol.ph \
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
	ABSBeamletStatsPSet.h

bin_PROGRAMS = BeamServer

BeamServer_SOURCES = \
	ABSBeamServerTask.h \
	ABSBeamServerTask.cc

BeamServer_LDADD = \
	$(top_builddir)/src/libabs.la -lpmllite -lsal -ltm -lgcfcommon

include $(lofar_sharedir)/Makefile.common
