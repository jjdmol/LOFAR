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
    log4cplus.properties

EXTRA_DIST = \
    log4cplus.properties

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	LogicalDeviceScheduler.conf \
	log4cplus.properties
	
MAC_CONFIG = $(top_srcdir)/../../Config
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
