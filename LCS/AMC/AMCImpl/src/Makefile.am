INSTHDRS			= ConverterImpl.h \
				  Exceptions.h \
				  AMCServer/ConverterProcess.h \
				  AMCServer/ConverterServer.h \
				  AMCServer/Process.h

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS				= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libamcimpl.la

libamcimpl_la_SOURCES 		= $(DOCHDRS) \
				  ConverterImpl.cc \
				  AMCServer/ConverterProcess.cc \
				  AMCServer/ConverterServer.cc \
				  AMCServer/Process.cc

bin_PROGRAMS			= amcserver

amcserver_SOURCES		= AMCServer/amcserver.cc
amcserver_LDADD			= libamcimpl.la
amcserver_DEPENDENCIES		= libamcimpl.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
