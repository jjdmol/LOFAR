noinst_LTLIBRARIES 	= libgcfceptest.la

libgcfceptest_la_DOCHDRS = \
	DH_EchoPing.h
	
libgcfceptest_la_SOURCES= $(libgcfceptest_la_DOCHDRS) \
	DH_EchoPing.cc

bin_PROGRAMS   		= CEPEcho CEPPing
  
CEPEcho_SOURCES							= CEPEcho.cc
CEPEcho_LDADD								= libgcfceptest.la
CEPEcho_DEPENDENCIES				= libgcfceptest.la $(LOFAR_DEPEND)

CEPPing_SOURCES							= CEPPing.cc
CEPPing_LDADD								= libgcfceptest.la
CEPPing_DEPENDENCIES				= libgcfceptest.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	mac.log_prop

EXTRA_DIST = \
	mac.log_prop

AM_CXXFLAGS=-fmessage-length=0
	
#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	mac.log_prop \
	typeStruct_TTypeF.dpl
	
MAC_CONFIG = $(top_srcdir)/../../../Config
%.log_prop: $(MAC_CONFIG)/%.log_prop
	cp $< $@
	
%.dpl: %.dpl.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

