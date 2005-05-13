INSTHDRS			= Converter.h \
				  EarthCoord.h \
				  Exceptions.h
				  SkyCoord.h \
				  TimeCoord.h \
				  AMCClient/BlobIO.h \
				  AMCClient/ConverterClient.h \
				  AMCClient/ConverterCommand.h \
				  AMCClient/DH_Converter.h

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS				= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libamcbase.la

libamcbase_la_SOURCES 		= $(DOCHDRS) \
				  EarthCoord.cc \
				  SkyCoord.cc \
				  TimeCoord.cc \
				  AMCClient/BlobIO.cc \
				  AMCClient/ConverterClient.cc \
				  AMCClient/ConverterCommand.cc \
				  AMCClient/DH_Converter.cc

include $(lofar_sharedir)/Makefile.common
