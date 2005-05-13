INSTHDRS			= ConverterImpl.h \
				  Exceptions.h \
				  AMCServer/ConverterServer.h

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS				= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libamcimpl.la

libamcimpl_la_SOURCES 		= $(DOCHDRS) \
				  ConverterImpl.cc \
				  AMCServer/ConverterServer.cc

include $(lofar_sharedir)/Makefile.common
