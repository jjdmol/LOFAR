AM_CPPFLAGS			= -DLOFARLOGGER_SUBPACKAGE=\"AMCClient\"

INSTHDRS			= BlobIO.h \
				  ConverterClient.h \
				  ConverterCommand.h \
				  DH_Converter.h

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS				= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libamcclient.la

libamcclient_la_SOURCES 	= $(DOCHDRS) \
				  BlobIO.cc \
				  ConverterClient.cc \
				  ConverterCommand.cc \
				  DH_Converter.cc

libamcclient_la_LIBADD		= ../libamcbase.la

include $(lofar_sharedir)/Makefile.common
