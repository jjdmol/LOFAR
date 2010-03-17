
check_PROGRAMS				= tPICtree \
							  tPICvalue \
							  tVICcomp \
							  tVTtree \
							  tVHtree \
							  tVHvalue \
							  tConnection \
							  tParamTypeConv \
							  tCampaign

tPICtree_SOURCES			= tPICtree.cc

tPICvalue_SOURCES			= tPICvalue.cc

tVICcomp_SOURCES			= tVICcomp.cc

tVTtree_SOURCES				= tVTtree.cc

tVHtree_SOURCES				= tVHtree.cc

tVHvalue_SOURCES			= tVHvalue.cc
tVHvalue_LDADD				= ../src/libotdb.la -lpqxx

tConnection_SOURCES			= tConnection.cc

tParamTypeConv_SOURCES		= tParamTypeConv.cc

tCampaign_SOURCES			= tCampaign.cc

LDADD						= ../src/libotdb.la -lpqxx
DEPENDENCIES				= ../src/libotdb.la $(LOFAR_DEPEND)

TESTSCRIPTS					= tParamTypeConv.sh \
							  tPICtree.sh \
							  tPICvalue.sh \
							  tVICcomp.sh \
							  tVTtree.sh \
							  tVHtree.sh \
							  tVHvalue.sh \
							  tConnection.sh \
							  tCampaign.sh

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
							  tParamTypeConv.log_prop \
							  tCampaign.log_prop \
							  $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common
