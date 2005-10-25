SUBDIRS						= . 

check_PROGRAMS				= tPICtree \
							  tPICvalue \
							  tVICcomp \
							  tVTtree \
							  tVHtree \
							  tVHvalue \
							  tConnection

tPICtree_SOURCES			= tPICtree.cc
tPICtree_LDADD				= ../src/libotdb.la -lpqxx
tPICtree_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tPICvalue_SOURCES			= tPICvalue.cc
tPICvalue_LDADD				= ../src/libotdb.la -lpqxx -laps
tPICvalue_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVICcomp_SOURCES			= tVICcomp.cc
tVICcomp_LDADD				= ../src/libotdb.la -lpqxx
tVICcomp_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVTtree_SOURCES				= tVTtree.cc
tVTtree_LDADD				= ../src/libotdb.la -lpqxx
tVTtree_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVHtree_SOURCES				= tVHtree.cc
tVHtree_LDADD				= ../src/libotdb.la -lpqxx
tVHtree_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tVHvalue_SOURCES			= tVHvalue.cc
tVHvalue_LDADD				= ../src/libotdb.la -lpqxx -laps
tVHvalue_DEPENDENCIES		= ../src/libotdb.la $(LOFAR_DEPEND)

tConnection_SOURCES			= tConnection.cc
tConnection_LDADD			= ../src/libotdb.la -lpqxx
tConnection_DEPENDENCIES	= ../src/libotdb.la $(LOFAR_DEPEND)


TESTS						= tPICtree_test.sh \
							  tPICvalue_test.sh \
							  tVICcomp_test.sh \
							  tVTtree_test.sh \
							  tVHtree_test.sh \
							  tVHvalue_test.sh \
							  tConnection_test.sh

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir)

include $(top_srcdir)/Makefile.common
