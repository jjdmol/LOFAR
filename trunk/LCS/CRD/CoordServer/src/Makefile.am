INSTHDRS =

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

bin_PROGRAMS 		= lofarcoordserver

lofarcoordserver_SOURCES	= lofarcoordserver.cc
lofarcoordserver_LDADD		= 
lofarcoordserver_DEPENDENCIES 	= $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
