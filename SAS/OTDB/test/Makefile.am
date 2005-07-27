SUBDIRS						= . 

check_PROGRAMS				= tConnection \
							  tPICadmin \
							  tOTDBinfo \
							  tVICadmin \
							  tVICcompfile

tConnection_SOURCES			= tConnection.cc
tConnection_LDADD			= ../src/libotdb.la -lpqxx
tConnection_DEPENDENCIES	= ../src/libotdb.la $(LOFAR_DEPEND)

tPICadmin_SOURCES			= tPICadmin.cc
tPICadmin_LDADD				= ../src/libotdb.la -lpqxx
tPICadmin_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVICadmin_SOURCES			= tVICadmin.cc
tVICadmin_LDADD				= ../src/libotdb.la -lpqxx
tVICadmin_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVICcompfile_SOURCES		= tVICcompfile.cc
tVICcompfile_LDADD			= ../src/libotdb.la -lpqxx
tVICcompfile_DEPENDENCIES	= ../src/libotdb.la $(LOFAR_DEPEND)

tOTDBinfo_SOURCES			= tOTDBinfo.cc
tOTDBinfo_LDADD				= ../src/libotdb.la -lpqxx
tOTDBinfo_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)


TESTS						= tConnection_test.sh \
							  tPICadmin_test.sh \
							  tVICadmin_test.sh \
							  tOTDBinfo_test.sh

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
