SUBDIRS						= . 

check_PROGRAMS				= tConnection

tConnection_SOURCES			= tConnection.cc
tConnection_LDADD			= ../src/libotdb.la -lpqxx
tConnection_DEPENDENCIES	= ../src/libotdb.la $(LOFAR_DEPEND)


TESTS						= tConnection.sh 

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
