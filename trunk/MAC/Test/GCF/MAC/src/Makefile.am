AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../../MACCommon/autogen $< && \
	chmod a+w $@

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

bin_PROGRAMS   		= Application1 Application2
  
Application1_SOURCES		    = Application1.cc
Application1_LDADD		      = libgcftest.la 
Application1_DEPENDENCIES		= libgcftest.la $(LOFAR_DEPEND)

Application2_SOURCES		    = Application2.cc
Application2_LDADD		      = libgcftest.la 
Application2_DEPENDENCIES		= libgcftest.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	TST_Protocol.ph \
	mac.log_prop

EXTRA_DIST = \
	TST_Protocol.ph \
	mac.log_prop
	
AM_CXXFLAGS=-fmessage-length=0
	
#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	Application1.conf \
	Application2.conf \
	mac.log_prop \
	customPrepPVSSDB.ctl

MAC_CONFIG = $(top_srcdir)/../../../Config
%.log_prop: $(MAC_CONFIG)/%.log_prop
	cp $< $@
	
%.conf: gcf-test.conf.in
	cp $< $@

%.ctl: *.ctl
	cp $< $@

include $(lofar_sharedir)/Makefile.common

