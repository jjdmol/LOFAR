DOCHDRS		 		= \
LofarDatabase.h	

lib_LTLIBRARIES 	= liblofardatabase.la

liblofardatabase_la_SOURCES 	= $(DOCHDRS)		\
LofarDatabase.cc

#bin_PROGRAMS		= ldb
#ldb_SOURCES 		= LofarDatabase.cc
#ldb_LDADD			= liblofardatabase.la
#ldb_DEPENDENCIES	= liblofardatabase.la $(LOFAR_DEPEND)

bin_PROGRAMS		= mdb
mdb_SOURCES 		= MySQLDatabase.cc
mdb_LDADD			= liblofardatabase.la
mdb_DEPENDENCIES	= liblofardatabase.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
