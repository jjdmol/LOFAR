DOCHDRS		 		= \
LofarDatabase.h	

lib_LTLIBRARIES 	= liblofardatabase.la 

liblofardatabase_la_SOURCES 	= $(DOCHDRS)		\
								  LofarDatabase.cc	\
								  MySQLDatabase.cc  \
								  PgSQLDatabase.cc

bin_PROGRAMS			= testAOD
testAOD_SOURCES 		= testAOD.cc
testAOD_LDADD			= liblofardatabase.la 
testAOD_DEPENDENCIES	= liblofardatabase.la $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
