DOCHDRS = \
	Collection.h \
	ConnectionHandler.h \
	Exception.h \
	ObjectCache.h \
	ObjecId.h \
	PersistenceBroker.h \
	PersistentObject.h \
	Query.h \
	SQLTimeStamp.h \
	TPersistentObject.h

lib_LTLIBRARIES = libpl.la

libpl_la_SOURCES = $(DOCHDRS) \
	ConnectionHandler.cc \
	ObjectCache.cc \
	ObjectId.cc \
	PersistenceBroker.cc \
	PersistentObject.cc

include $(lofar_sharedir)/Makefile.common
