
AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../../GCF/GCFCommon/autogen $< && \
	chmod a+w $@

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

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

AM_CXXFLAGS = -I$(top_srcdir)/src

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
	ABSBeamlet.cc

bin_PROGRAMS = BeamServer

BeamServer_SOURCES = \
	ABSBeamServerTask.h \
	ABSBeamServerTask.cc

BeamServer_LDADD = \
	$(top_builddir)/src/libabs.la -ltm -lgcfcommon

include $(lofar_sharedir)/Makefile.common