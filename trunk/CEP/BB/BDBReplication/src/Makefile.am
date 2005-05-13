lib_LTLIBRARIES	= libBDBReplication.la

DOCHDRS 	=				\
BDBSite.h					\
BDBReplicator.h					\
BDBConHandler.h					\
BDBListener.h

COMMONSRC 		= $(DOCHDRS)		\
tBDBReplication.cc		   		\
BDBReplicator.cc				\
BDBConHandler.cc				\
BDBListener.cc					\
BDBSite.cc


libBDBReplication_la_SOURCES  	= $(COMMONSRC)


include $(lofar_sharedir)/Makefile.common
