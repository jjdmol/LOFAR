bin_PROGRAMS 		= RegisterAccess

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen  $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen $<

RegisterAccess_CPPFLAGS= \
	-I$(top_srcdir)/APLCommon/src \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

RegisterAccess_SOURCES 		= \
	ARAMain.cc \
	ARAAnswer.cc \
	ARAPhysicalModel.cc \
	ARARegisterAccessTask.cc
	
RegisterAccess_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la \
	-lrsp \
	-lgcfcommon
	
RegisterAccess_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
    ara.ns \
    ara.top \
    log4cplus.properties	\
    registeraccess.conf

EXTRA_DIST = \
    ara.ns \
    ara.top \
    log4cplus.properties

sysconf_DATA = \
	ara.ns \
	ara.top \
	registeraccess.conf
	
%.properties: %.properties.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
