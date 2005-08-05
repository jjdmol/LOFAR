SUBDIRS						= . 

check_PROGRAMS				= tPICadmin \
							  tVICcomp \
							  tVTtree \
							  tVICadmin \
							  tOTDBinfo \
							  tConnection

tConnection_SOURCES			= tConnection.cc
tConnection_LDADD			= ../src/libotdb.la -lpqxx
tConnection_DEPENDENCIES	= ../src/libotdb.la $(LOFAR_DEPEND)

tPICadmin_SOURCES			= tPICadmin.cc
tPICadmin_LDADD				= ../src/libotdb.la -lpqxx
tPICadmin_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVICcomp_SOURCES			= tVICcomp.cc
tVICcomp_LDADD				= ../src/libotdb.la -lpqxx
tVICcomp_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVTtree_SOURCES				= tVTtree.cc
tVTtree_LDADD				= ../src/libotdb.la -lpqxx
tVTtree_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVICadmin_SOURCES			= tVICadmin.cc
tVICadmin_LDADD				= ../src/libotdb.la -lpqxx
tVICadmin_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tOTDBinfo_SOURCES			= tOTDBinfo.cc
tOTDBinfo_LDADD				= ../src/libotdb.la -lpqxx
tOTDBinfo_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)


TESTS						= tPICadmin_test.sh \
							  tVICadmin_test.sh \
							  tOTDBinfo_test.sh \
							  tConnection_test.sh

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
