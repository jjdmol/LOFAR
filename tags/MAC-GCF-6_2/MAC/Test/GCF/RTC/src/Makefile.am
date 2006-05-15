AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	
DOCHDRS	=	\
  RTAnswer.h \
  RTDefines.h

noinst_LTLIBRARIES 	= librtc.la

librtc_la_SOURCES= $(DOCHDRS) \
 	RTAnswer.cc \
	Echo_Protocol.cc

bin_PROGRAMS   		= RTEcho RTPing
  
RTEcho_SOURCES							= RTEcho.cc
RTEcho_LDADD								= librtc.la
RTEcho_DEPENDENCIES					= librtc.la $(LOFAR_DEPEND)

RTPing_SOURCES							= RTPing.cc
RTPing_LDADD								= librtc.la
RTPing_DEPENDENCIES					= librtc.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	Echo_Protocol.ph

EXTRA_DIST = $(BUILT_SOURCES)
	
AM_CXXFLAGS=-fmessage-length=0
	
#in case of make install these files will be copied to the bindir beside the test apps
sysconf_DATA= \
	RTEcho.conf \
	RTPing.conf
	
%.conf: gcf-test.conf.in
	cp $< $@

clean-local:
	rm -f *.ph
  
include $(top_srcdir)/Makefile.common

