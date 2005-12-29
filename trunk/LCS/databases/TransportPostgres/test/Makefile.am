check_PROGRAMS        = ExampleDB ExampleDB2

# programs to run through supplied checktools
CHECKTOOLPROGS        =
#ENDCHECKTOOLPROGS

# scripts used to run tests
TESTSCRIPTS           = ExampleDB_test.sh \
			ExampleDB2_test.sh

# scripts and possible programs to run tests
TESTS	              = $(TESTSCRIPTS)

XFAIL_TESTS           =

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST            = $(TESTSCRIPTS)

# Lines to build a test program testprg
ExampleDB_SOURCES               = ExampleDB.cc DH_ExampleExtra3.cc

ExampleDB2_SOURCES              = ExampleDB2.cc DH_ExampleExtra3.cc

LDADD			= ../src/libtransportpostgres.la
DEPENDENCIES		= ../src/libtransportpostgres.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  srcdir=$(srcdir) \
				  LOFARROOT=$(LOFARROOT) \
				  LOFARDATAROOT=$(LOFARDATAROOT) \
				  MPIBIN=$(MPIBIN)


include $(top_srcdir)/Makefile.common
