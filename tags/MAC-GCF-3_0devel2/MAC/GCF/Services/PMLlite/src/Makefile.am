pkginclude_HEADERS = \
  GCF/GCF_RTAnswer.h \
  GCF/GCF_RTMyProperty.h \
  GCF/GCF_RTMyPropertySet.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
  GPM_RTController.h \
  GPM_RTDefines.h
    
noinst_LTLIBRARIES 	= libpmllite.la

libpmllite_la_SOURCES= $(DOCHDRS) \
  GCF_RTAnswer.cc \
  GCF_RTMyProperty.cc \
  GCF_RTMyPropertySet.cc \
  GPM_RTController.cc
  
libpmllite_la_CPPFLAGS= -I$(top_srcdir)/GCFCommon/src \
	-I$(top_srcdir)/CoreComps/TM/src \
	-I$(top_srcdir)/CoreComps/SAL/src \
	-I$(top_srcdir)/Services/PI/src \
	-I$(top_builddir)/Services/SupServer/src \
	-I$(top_srcdir)/Services/PMLlite/src/GCF

include $(lofar_sharedir)/Makefile.common

