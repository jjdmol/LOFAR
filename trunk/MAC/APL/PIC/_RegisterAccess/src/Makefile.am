noinst_PROGRAMS 		= ARARegisterAccess

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen  -L $(top_builddir)/PAC/RSPDriver/src $<

ARARegisterAccess_CPPFLAGS= \
	-I$(top_srcdir)/PAC/RSPDriver/src \
	-I$(top_srcdir)/APLCommon/src \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

ARARegisterAccess_SOURCES 		= \
	ARAMain.cc \
	ARAAnswer.cc \
	ARARegisterAccessTask.cc \
	RSP_Protocol.cc
	
ARARegisterAccess_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la \
	-lrsp \
	-lgcfcommon
	
ARARegisterAccess_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
  $(top_builddir)/PAC/RSPDriver/src/RSP_Protocol.ph \
    ara.ns \
    ara.top \
    log4cplus.properties	

EXTRA_DIST = \
  $(top_builddir)/PAC/RSPDriver/src/RSP_Protocol.ph \
    ara.ns \
    ara.top \
    log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
