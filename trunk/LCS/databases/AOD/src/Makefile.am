DOCHDRS		 		= \
LofarDatabase.h	

lib_LTLIBRARIES 	= liblofardatabase.la 

liblofardatabase_la_SOURCES 	= $(DOCHDRS)		\
								  LofarDatabase.cc	\
								  MySQLDatabase.cc  \
								  PgSQLDatabase.cc

bin_PROGRAMS			= testAOD testDTL testOTL updateDTL \
						  DTLdemo_no_index DTLdemo_indexed

### testAOD ###
testAOD_SOURCES 		= testAOD.cc
testAOD_LDADD			= -L/usr/local/lib/pqxx		\
						  -lsqlplus -lpqxx -lpq liblofardatabase.la 
testAOD_DEPENDENCIES	= liblofardatabase.la $(LOFAR_DEPEND)

### testDTL ###
testDTL_SOURCES 		= testDTL.cc
testDTL_CXXFLAGS		= -I../../../src/DTL/lib
testDTL_LDADD			= -L../../../src/DTL/lib/release -lDTL -lodbc
testDTL_DEPENDENCIES	= $(LOFAR_DEPEND)

### testOTL ###
testOTL_SOURCES 		= testOTL.cc
testOTL_CXXFLAGS		= -I../../../src/OTL
testOTL_LDADD			= -lodbc
testOTL_DEPENDENCIES	= $(LOFAR_DEPEND)

### updateDTL ###
updateDTL_SOURCES 		= updateDTL.cc
updateDTL_CXXFLAGS		= -I../../../src/DTL/lib
updateDTL_LDADD			= -L../../../src/DTL/lib/release -lDTL -lodbc
updateDTL_DEPENDENCIES	= $(LOFAR_DEPEND)

### DTLdemo_indexed ###
DTLdemo_indexed_SOURCES 		= DTLdemo_indexed.cc
DTLdemo_indexed_CXXFLAGS		= -I../../../src/DTL/lib
DTLdemo_indexed_LDADD			= -L../../../src/DTL/lib/release -lDTL -lodbc
DTLdemo_indexed_DEPENDENCIES	= $(LOFAR_DEPEND)

### DTLdemo_no_index ###
DTLdemo_no_index_SOURCES 		= DTLdemo_no_index.cc
DTLdemo_no_index_CXXFLAGS		= -I../../../src/DTL/lib
DTLdemo_no_index_LDADD			= -L../../../src/DTL/lib/release -lDTL -lodbc
DTLdemo_no_index_DEPENDENCIES	= $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
