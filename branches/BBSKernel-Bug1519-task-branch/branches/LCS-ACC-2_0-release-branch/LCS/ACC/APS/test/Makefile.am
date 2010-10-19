
check_PROGRAMS				= tParameterSet 

TESTS						= tParameterSet_test.sh

### tParameterSet ###
tParameterSet_SOURCES		= tParameterSet.cc
tParameterSet_DEPENDENCIES	= $(LOFAR_DEPEND) ../src/libaps.la
tParameterSet_LDADD			= ../src/libaps.la

TESTS_ENVIRONMENT			= lofar_sharedir=$(lofar_sharedir) \
							  LOFARROOT=$(LOFARROOT)

include $(lofar_sharedir)/Makefile.common

