AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../GCF/GCFCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
	SupervisedTask.h \
  	PropertyProxy.h \
  	Answer.h

noinst_LTLIBRARIES 	= libgcftest.la

libgcftest_la_SOURCES= $(DOCHDRS) \
	SupervisedTask.cc \
 	PropertyProxy.cc \
 	Answer.cc

noinst_PROGRAMS   		= Application1 Application2
  
Application1_SOURCES		    = Application1.cc
Application1_LDADD		      = libgcftest.la 
Application1_DEPENDENCIES	= libgcftest.la $(LOFAR_DEPEND)

Application2_SOURCES		    = Application2.cc
Application2_LDADD		      = libgcftest.la 
Application2_DEPENDENCIES	= libgcftest.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	TST_Protocol.ph \
	test.ns \
	test.top \
	log4cplus.properties

EXTRA_DIST = \
	TST_Protocol.ph \
	test.ns \
	test.top \
	log4cplus.properties
	
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@
	
%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

