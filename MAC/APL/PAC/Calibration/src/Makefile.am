

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
	CalServer.conf \
	CAL_Protocol.ph

EXTRA_DIST = \
	CAL_Protocol.ph

sysconf_DATA = \
	CalServer.conf

include_HEADERS = \
	$(top_builddir)/src/CAL_Protocol.ph

%.properties: %.properties.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

lib_LTLIBRARIES = libcal.la

libcal_la_SOURCES = \
	CalInterface.h \
	CalAlgorithm.h \
	CalAlgorithm.cc \
	$(top_builddir)/src/CAL_Protocol.cc

bin_PROGRAMS = CalServer

CalServer_SOURCES = \
	CalServer.h \
	CalServer.cc

CalServer_LDADD = \
	$(top_builddir)/src/libcal.la -lrsp

include $(lofar_sharedir)/Makefile.common
