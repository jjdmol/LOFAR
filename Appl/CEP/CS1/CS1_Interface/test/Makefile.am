check_PROGRAMS		= tSparseSet

# programs to run through supplied checktools
CHECKTOOLPROGS		= tSparseSet
#ENDCHECKTOOLPROGS

# scripts used to run tests
TESTSCRIPTS		=

# scripts and possible programs to run tests
TESTS			= $(TESTSCRIPTS) \
			  tSparseSet

XFAIL_TESTS		=

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST		= $(TESTSCRIPTS)

tSparseSet_SOURCES	= tSparseSet.cc
tSparseSet_LDADD	= ../src/libcs1_interface.la
tSparseSet_DEPENDENCIES = ../src/libcs1_interface.la

# Lines to build a test program testprg
#testprg_SOURCES		= testprg.cc
#testprg_LDADD			= ../src/libcs1_interface.la
#testprg_DEPENDENCIES		= ../src/libcs1_interface.la $(LOFAR_DEPEND)



include $(top_srcdir)/Makefile.common
