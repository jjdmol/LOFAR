INSTHDRS 				  = ProcessControl.h \
							ProcControlServer.h \
							DH_ProcControl.h \
							ProcControlComm.h

NOINSTHDRS 				  =

TCCHDRS 				  =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 			  = $(NOINSTHDRS)

DOCHDRS				      = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES           = libplc.la

libplc_la_SOURCES 		  = DH_ProcControl.cc \
							ProcControlComm.cc  \
							ProcControlServer.cc  

include $(lofar_sharedir)/Makefile.common

