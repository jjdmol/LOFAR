check_PROGRAMS			= tDebug # tLorrays

tDebug_SOURCES			= tDebug.cc
tDebug_LDADD			= ../src/libcommon.la
tDebug_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

# tLorrays_SOURCES		= tLorrays.cc
# tLorrays_LDADD		= ../src/libcommon.la	
# tLoarrays_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)


TESTS	=					\
	tDebug_test.sh 
# tLorrays_test.sh				

EXTRA_DIST = 					\
	tDebug_test.sh tLorrays_test.sh


include $(lofar_sharedir)/Makefile.common
