AUTOGEN = autogen
SUFFIXES = .ph
%.ph: SS/%.prot
	$(AUTOGEN) -L $(top_srcdir)/GCFCommon/autogen $< && \
	chmod a+w $@

DOCHDRS	=	\
  GCF_RTAnswer.h \
  GCF_RTMyProperty.h \
  GCF_RTMyPropertySet.h \
  GPM_RTController.h \
  GPM_RTDefines.h
    
lib_LTLIBRARIES 	= libpmllite.la

libpmllite_la_SOURCES= $(DOCHDRS) \
  GCF_RTAnswer.cc \
  GCF_RTMyProperty.cc \
  GCF_RTMyPropertySet.cc \
  GPM_RTController.cc
  
libpmllite_la_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src \
	-I$(top_srcdir)/Services/PI/src \
	-I$(top_builddir)/Services/PI/src

include $(lofar_sharedir)/Makefile.common

