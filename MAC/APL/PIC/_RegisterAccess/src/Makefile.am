noinst_PROGRAMS 		= ARARegisterAccess

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../MACCommon/autogen $<

ARARegisterAccess_CPPFLAGS= \
	-I$(top_srcdir)/APLCommon/src \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

ARARegisterAccess_SOURCES 		= \
	ARAMain.cc \
	ARARegisterAccessTask.cc \
	ARATest_Protocol.cc
	
ARARegisterAccess_LDADD		= \
	$(top_builddir)/APLCommon/src/libaplcommon.la
ARARegisterAccess_DEPENDENCIES	= \
	$(top_builddir)/APLCommon/src/libaplcommon.la

BUILT_SOURCES = \
    ARATest_Protocol.ph \
	log4cplus.properties	

EXTRA_DIST = \
    ARATest_Protocol.ph \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
