common_top_builddir=@common_top_builddir@

check_PROGRAMS			= Example ExampleMem ExamplePL ExamplePL2 ExamplePL3 ExamplePL4 TestBidirectional

Example_SOURCES 		= Example.cc DH_Example.cc

ExampleMem_SOURCES 		= ExampleMem.cc DH_Example.cc

ExamplePL_SOURCES 		= ExamplePL.cc DH_Example.cc

ExamplePL2_SOURCES 		= ExamplePL2.cc DH_Example2.cc

ExamplePL3_SOURCES 		= ExamplePL3.cc DH_Example2.cc

ExamplePL4_SOURCES 		= ExamplePL4.cc DH_Example2.cc

TestBidirectional_SOURCES 	= TestBidirectional.cc DH_Example.cc

LDADD				= ../src/libtransportpl.la 
DEPENDENCIES			= ../src/libtransportpl.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir) \
				  MPIBIN=$(MPIBIN)

TESTS	=					\
	Example_test.sh				\
	ExampleMem_test.sh			\
	ExamplePL_test.sh			\
	ExamplePL2_test.sh			\
	ExamplePL3_test.sh			\
	ExamplePL4_test.sh			\
	TestBidirectional_test.sh		


include $(lofar_sharedir)/Makefile.common
