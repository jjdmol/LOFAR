AUTOGEN = autogen
SUFFIXES = .ph
%.ph: $(top_srcdir)/PA/src/%.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
	GPA_ExternHdl.h \
	PA_Protocol.ph
   
lib_LTLIBRARIES 	= libgcfpalext.la

libgcfpalext_la_LIBADD= 

libgcfpalext_la_LDFLAGS= -shared
libgcfpalext_la_DEPENDENCIES= $(LOFAR_DEPEND)

libgcfpalext_la_SOURCES= $(DOCHDRS) \
	GPA_ExternHdl.cc \
	PA_Protocol.cc

libgcfpalext_la_CPPFLAGS= -I$(top_srcdir)/include \
	-I$(top_srcdir)/SAL/src \
	-I$(top_srcdir)/PA/src \
	-I$(API_ROOT)/include/Ctrl
	
BUILT_SOURCES = \
	PA_Protocol.ph
	
EXTRA_DIST = \
	PA_Protocol.ph

%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

