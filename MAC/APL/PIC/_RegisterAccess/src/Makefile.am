noinst_PROGRAMS 		= ARARegisterAccess

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen  $<

ARARegisterAccess_CPPFLAGS= \
	-I$(top_srcdir)/PAC/RSPDriver/src \
	-I$(top_srcdir)/APLCommon/src \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

ARARegisterAccess_SOURCES 		= \
	ARAMain.cc \
	ARAAnswer.cc \
	ARAPhysicalModel.cc \
	ARARegisterAccessTask.cc
	
ARARegisterAccess_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la \
	$(top_srcdir)/PAC/RSPDriver/build/gnu_debug/src/librsp.la \
	-lgcfcommon
	
ARARegisterAccess_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
    ara.ns \
    ara.top \
    log4cplus.properties	

EXTRA_DIST = \
    ara.ns \
    ara.top \
    log4cplus.properties
	
%.properties: %.properties.in
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
