
check_PROGRAMS				= tPICtree \
							  tPICvalue \
							  tVICcomp \
							  tVTtree \
							  tVHtree \
							  tVHvalue \
							  tConnection \
							  tConverter

tPICtree_SOURCES			= tPICtree.cc

tPICvalue_SOURCES			= tPICvalue.cc

tVICcomp_SOURCES			= tVICcomp.cc

tVTtree_SOURCES				= tVTtree.cc

tVHtree_SOURCES				= tVHtree.cc

tVHvalue_SOURCES			= tVHvalue.cc
tVHvalue_LDADD				= ../src/libotdb.la -lpqxx

tConnection_SOURCES			= tConnection.cc

tConverter_SOURCES			= tConverter.cc

LDADD						= ../src/libotdb.la -lpqxx
DEPENDENCIES				= ../src/libotdb.la $(LOFAR_DEPEND)

TESTSCRIPTS					= tConverter_test.sh \
							  tPICtree_test.sh \
							  tPICvalue_test.sh \
							  tVICcomp_test.sh \
							  tVTtree_test.sh \
							  tVHtree_test.sh \
							  tVHvalue_test.sh \
							  tConnection_test.sh

TESTS						= $(TESTSCRIPTS)


EXTRA_DIST					= DATABASENAME \
							  tConnection.log_prop \
							  tPICtree.in \
							  tPICtree.log_prop \
							  tPICvalue.in \
							  tPICvalue.log_prop \
							  tVHtree.log_prop \
							  tVHvalue.in \
							  tVHvalue.log_prop \
							  tVICcomp.in \
							  tVICcomp.log_prop \
							  tVTtree.in \
							  tVTtree.log_prop \
							  tConverter.log_prop \
							  $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common
