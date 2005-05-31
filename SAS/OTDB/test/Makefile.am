SUBDIRS						= . 

check_PROGRAMS				= tConnection tPICadmin

tConnection_SOURCES			= tConnection.cc
tConnection_LDADD			= ../src/libotdb.la -lpqxx
tConnection_DEPENDENCIES	= ../src/libotdb.la $(LOFAR_DEPEND)

tPICadmin_SOURCES			= tPICadmin.cc
tPICadmin_LDADD				= ../src/libotdb.la -lpqxx
tPICadmin_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)


TESTS						= tConnection_test.sh tPICadmin_test.sh

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
