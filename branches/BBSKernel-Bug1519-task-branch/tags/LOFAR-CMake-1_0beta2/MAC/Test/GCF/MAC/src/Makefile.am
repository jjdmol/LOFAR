AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	
libgcftest_la_DOCHDRS	=	\
	SupervisedTask.h \
  PropertyProxy.h \
  Answer.h \
  Defines.h

noinst_LTLIBRARIES 	= libgcftest.la

libgcftest_la_SOURCES= $(libgcftest_la_DOCHDRS) \
	SupervisedTask.cc \
 	PropertyProxy.cc \
 	Answer.cc \
 	TST_Protocol.cc

bin_PROGRAMS   		= Application1 Application2 Application3
  
Application1_SOURCES		    = Application1.cc
Application1_LDADD		      = libgcftest.la 
Application1_DEPENDENCIES		= libgcftest.la $(LOFAR_DEPEND)

Application2_SOURCES		    = Application2.cc
Application2_LDADD		      = libgcftest.la 
Application2_DEPENDENCIES		= libgcftest.la $(LOFAR_DEPEND)

Application3_SOURCES		    = Application3.cc
Application3_LDADD		      = libgcftest.la 
Application3_DEPENDENCIES		= libgcftest.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	TST_Protocol.ph

EXTRA_DIST = $(BUILT_SOURCES)
	
AM_CXXFLAGS=-fmessage-length=0
	
#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	customPrepPVSSDB.ctl

sysconf_DATA= \
	Application1.conf \
	Application2.conf \
	Application3.conf

%.conf: gcf-test.conf.in
	cp $< $@

%.ctl: *.ctl
	cp $< $@

clean-local:
	rm -f *.ph
  
include $(top_srcdir)/Makefile.common

