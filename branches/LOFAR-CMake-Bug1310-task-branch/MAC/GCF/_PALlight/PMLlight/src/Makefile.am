DOCHDRS	=	\
  GPM_RTController.h \
  GPM_RTDefines.h
        
noinst_LTLIBRARIES 	= libpmllight.la

libpmllight_la_SOURCES= $(DOCHDRS) \
  GCF_RTAnswer.cc \
  GCF_RTMyProperty.cc \
  GCF_RTMyPropertySet.cc \
  GPM_RTController.cc
  
lib_LTLIBRARIES 	= libgcfpallight.la

libgcfpallight_la_SOURCES=
libgcfpallight_la_LIBADD= libpmllight.la

libgcfpallight_la_DEPENDENCIES= libpmllight.la $(LOFAR_DEPEND)

AM_CPPFLAGS = -DLOFARLOGGER_SUBPACKAGE=\"RTC\"

EXTRA_DIST= $(configfiles_DATA) $(sysconf_DATA)

sysconf_DATA= \
	gcf-pmllight.conf
	
configfilesdir=$(bindir)
configfiles_DATA= \
	genTypeInfo

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common