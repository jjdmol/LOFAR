DOCHDRS	= \
ParameterSet.h 

lib_LTLIBRARIES                 = libacc.la

libacc_la_SOURCES = $(DOCHDRS) \
ParameterSet.cc

bin_PROGRAMS			= tParameterSet 

### tParameterSet ###
tParameterSet_SOURCES 		= tParameterSet.cc
tParameterSet_DEPENDENCIES	= $(LOFAR_DEPEND)
tParameterSet_LDADD		= libacc.la

include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
