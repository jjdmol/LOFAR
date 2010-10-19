INSTHDRS 				  = ApplControl.h \
							ApplControlServer.h \
							ApplControlClient.h \
							ACAsyncClient.h \
							ACSyncClient.h \
							ApplControlComm.h \
							DH_ApplControl.h \
							ACRequest.h

NOINSTHDRS 				  =

TCCHDRS 				  =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 			  = $(NOINSTHDRS)

DOCHDRS					  = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES           = libalc.la

libalc_la_SOURCES 		  = DH_ApplControl.cc \
							ApplControlComm.cc \
							ApplControlClient.cc \
							ACSyncClient.cc  \
							ACAsyncClient.cc \
							ApplControlServer.cc 

include $(top_srcdir)/Makefile.common

