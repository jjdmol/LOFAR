AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/../../GCF/GCFCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
	SupervisedTask.h \
  PropertyProxy.h

lib_LTLIBRARIES 	= libgcf.la

libgcf_la_SOURCES= $(DOCHDRS) \
	SupervisedTask.cc \
  PropertyProxy.cc

bin_PROGRAMS   		= Application1 Application2
  
Application1_SOURCES		    = Application1.cc
Application1_LDADD		      = libgcf.la 
Application1_DEPENDENCIES	= libgcf.la $(LOFAR_DEPEND)

Application2_SOURCES		    = Application2.cc
Application2_LDADD		      = libgcf.la 
Application2_DEPENDENCIES	= libgcf.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	TST_Protocol.ph

EXTRA_DIST = \
	TST_Protocol.ph \
	test.ns \
	test.top \
	log4cplus.properties
	
pkgdata_DATA = \
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

