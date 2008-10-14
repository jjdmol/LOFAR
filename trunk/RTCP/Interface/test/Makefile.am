check_PROGRAMS		= tSparseSet tRSPTimeStamp

# programs to run through supplied checktools
CHECKTOOLPROGS		= tSparseSet tRSPTimeStamp
#ENDCHECKTOOLPROGS

# scripts used to run tests
TESTSCRIPTS		=

# scripts and possible programs to run tests
TESTS			= $(TESTSCRIPTS) \
			  tSparseSet \
			  tRSPTimeStamp

XFAIL_TESTS		=

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST		= $(TESTSCRIPTS)

tSparseSet_SOURCES	= tSparseSet.cc
tSparseSet_LDADD	= ../src/libinterface.la
tSparseSet_DEPENDENCIES = ../src/libinterface.la

tRSPTimeStamp_SOURCES	= tRSPTimeStamp.cc
tRSPTimeStamp_LDADD	= ../src/libinterface.la
tRSPTimeStamp_DEPENDENCIES = ../src/libinterface.la

# Lines to build a test program testprg
#testprg_SOURCES		= testprg.cc
#testprg_LDADD			= ../src/libinterface.la
#testprg_DEPENDENCIES		= ../src/libinterface.la $(LOFAR_DEPEND)



include $(top_srcdir)/Makefile.common
