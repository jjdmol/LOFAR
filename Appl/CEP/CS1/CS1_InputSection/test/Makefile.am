check_PROGRAMS        =

# programs to run through supplied checktools
CHECKTOOLPROGS        =
#ENDCHECKTOOLPROGS

# scripts used to run tests
TESTSCRIPTS           =

# scripts and possible programs to run tests
TESTS	              = $(TESTSCRIPTS)

XFAIL_TESTS           =

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST            = $(TESTSCRIPTS)

# Lines to build a test program testprg
#testprg_SOURCES		= testprg.cc
#testprg_LDADD			= ../src/libcs1_inputsection.la
#testprg_DEPENDENCIES		= ../src/libcs1_inputsection.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  srcdir=$(srcdir) \
				  LOFARROOT=$(LOFARROOT) \
				  LOFARDATAROOT=$(LOFARDATAROOT) \
				  MPIBIN=$(MPIBIN)


include $(top_srcdir)/Makefile.common
