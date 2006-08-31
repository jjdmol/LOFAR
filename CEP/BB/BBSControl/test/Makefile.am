check_PROGRAMS			= tBBSStrategy

# programs to run through supplied checktools
CHECKTOOLPROGS			=
#ENDCHECKTOOLPROGS

TESTSCRIPTS			= tBBSStrategy.sh

TESTS				= $(TESTSCRIPTS)

EXTRA_DIST			= $(TESTSCRIPTS)

tBBSStrategy_SOURCES		= tBBSStrategy.cc
tBBSStrategy_LDADD		= ../src/libbbscontrol.la
tBBSStrategy_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT)

include $(top_srcdir)/Makefile.common
