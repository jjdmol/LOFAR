
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
	RSP_Protocol.ph \
	EPA_Protocol.ph

# 	log4cplus.properties \
# 	abs.ns \
# 	abs.top

EXTRA_DIST = \
	RSP_Protocol.ph \
	EPA_Protocol.ph

# 	log4cplus.properties \
# 	abs.ns \
# 	abs.top

include_HEADERS = \
	RSP_Protocol.ph \
	EPA_Protocol.ph

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

noinst_LTLIBRARIES = librsp.la libepa.la

librsp_la_SOURCES = \
	Timestamp.h \
	Timestamp.cc \
	BeamletWeights.h \
	BeamletWeights.cc \
	SubbandSelection.h \
	SubbandSelection.cc \
	RCUSettings.h \
	RCUSettings.cc \
	WGSetting.h \
	WGSetting.cc \
	SystemStatus.h \
	SystemStatus.cc \
	Statistics.h \
	Statistics.cc \
	Versions.h \
	Versions.cc \
	RSP_Protocol.cc

libepa_la_SOURCES = \
	EPA_Protocol.cc

bin_PROGRAMS = RSPDriver

RSPDriver_SOURCES = \
	RSPDriverTask.h \
	RSPDriverTask.cc \
	Scheduler.h \
	Scheduler.cc \
	Command.h \
	Command.cc \
	SyncAction.h \
	SyncAction.cc

RSPDriver_LDADD = \
	$(top_builddir)/src/librsp.la \
	$(top_builddir)/src/libepa.la \
	-lgcf

include $(lofar_sharedir)/Makefile.common
