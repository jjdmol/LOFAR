AM_CPPFLAGS			= -DLOFARLOGGER_SUBPACKAGE=\"AMCServer\"

INSTHDRS			= 

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS				= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libamcserver.la

libamcserver_la_SOURCES 	= $(DOCHDRS)

libamcserver_la_LIBADD		= ../libamcimpl.la

include $(lofar_sharedir)/Makefile.common
