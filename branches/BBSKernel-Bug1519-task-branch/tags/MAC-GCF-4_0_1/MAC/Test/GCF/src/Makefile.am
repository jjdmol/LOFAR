AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
	chmod a+w $@

libgcftest_la_DOCHDRS	=	\
	SupervisedTask.h \
  PropertyProxy.h \
  Answer.h \
  Defines.h

libgcfrttest_la_DOCHDRS	=	\
  RTAnswer.h \
  RTDefines.h

noinst_LTLIBRARIES 	= libgcftest.la libgcfrttest.la

libgcftest_la_SOURCES= $(libgcftest_la_DOCHDRS) \
	SupervisedTask.cc \
 	PropertyProxy.cc \
 	Answer.cc \
 	TST_Protocol.cc

libgcfrttest_la_SOURCES= $(libgcfrttest_la_DOCHDRS) \
 	RTAnswer.cc \
	Echo_Protocol.cc

bin_PROGRAMS   		= Application1 Application2 RTEcho RTPing
  
Application1_SOURCES		    = Application1.cc
Application1_LDADD		      = libgcftest.la 
Application1_DEPENDENCIES		= libgcftest.la $(LOFAR_DEPEND)

Application2_SOURCES		    = Application2.cc
Application2_LDADD		      = libgcftest.la 
Application2_DEPENDENCIES		= libgcftest.la $(LOFAR_DEPEND)

RTEcho_SOURCES							= RTEcho.cc
RTEcho_LDADD								= libgcfrttest.la
RTEcho_DEPENDENCIES					= libgcfrttest.la $(LOFAR_DEPEND)

RTPing_SOURCES							= RTPing.cc
RTPing_LDADD								= libgcfrttest.la
RTPing_DEPENDENCIES					= libgcfrttest.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	TST_Protocol.ph \
	Echo_Protocol.ph \
	test.ns \
	test.top \
	log4cplus.properties

EXTRA_DIST = \
	TST_Protocol.ph \
	Echo_Protocol.ph \
	test.ns \
	test.top \
	log4cplus.properties
	
#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	test.ns \
	test.top \
	log4cplus.properties
	
%.properties: $(top_srcdir)/../../Config/%.properties
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

