DOCHDRS = Collection.h \
          ConnectionHandler.h \
          Exception.h \
          ObjecId.h \
          PersistenceBroker.h \
          PersistentObject.h \
          Query.h \
          SQLTimeStamp.h \
          TPersistentObjectBase.h \
          TPersistentObject.h

lib_LTLIBRARIES = libpl.la

libpl_la_SOURCES = $(DOCHDRS) \
	  ConnectionHandler.cc \
	  ObjectId.cc \
	  PersistenceBroker.cc

include $(lofar_sharedir)/Makefile.common
