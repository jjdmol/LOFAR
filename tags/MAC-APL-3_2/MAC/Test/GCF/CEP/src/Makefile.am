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

AM_CXXFLAGS=-fmessage-length=0
	
#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	typeStruct_TTypeF.dpl
	
%.dpl: %.dpl.in
	cp $< $@

include $(top_srcdir)/Makefile.common

