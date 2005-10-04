INSTHDRS          = BDBMessage.h \
BDBSite.h					\
BDBReplicator.h					\
BDBCommunicator.h					\
BDBConnector.h

NOINSTHDRS =

TCCHDRS =

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES	= libbdbreplication.la

libbdbreplication_la_SOURCES  	 = BDBMessage.cc \
BDBReplicator.cc				\
BDBCommunicator.cc				\
BDBConnector.cc					\
BDBSite.cc

include $(top_srcdir)/Makefile.common
