SUBDIRS				= . AMCServer

INSTHDRS			= ConverterImpl.h \
				  Exceptions.h

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS				= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libamcimpl.la

libamcimpl_la_SOURCES 		= $(DOCHDRS) \
				  ConverterImpl.cc

include $(lofar_sharedir)/Makefile.common
