AM_CPPFLAGS = \
	-DHAVE_MMAP=0 \
	-DMORECORE=shmbrk \
	-DMORECORE_CONTIGUOUS=0 \
	-DMORECORE_CANNOT_TRIM=1 \
	-DSHMEM_ALLOC

check_PROGRAMS			= Example TestBidirectional ExampleMem ExampleBlMem ExampleNonBlocking ExampleShMem ExampleMPI ExampleVarBuf ExampleSocket ExampleSocketPerf tTH_Socket

Example_SOURCES 		= Example.cc DH_Example.cc

ExampleMem_SOURCES 		= ExampleMem.cc DH_Example.cc

ExampleBlMem_SOURCES		= ExampleBlMem.cc DH_Example.cc

ExampleNonBlocking_SOURCES      = ExampleNonBlocking.cc DH_Example.cc

ExampleShMem_SOURCES 		= ExampleShMem.cc DH_Example.cc

ExampleMPI_SOURCES 		= ExampleMPI.cc DH_Example.cc

ExampleVarBuf_SOURCES           = ExampleVarBuf.cc DH_VarBuf.cc

ExampleSocket_SOURCES		= ExampleSocket.cc DH_Example.cc

ExampleSocketPerf_SOURCES	= ExampleSocketPerf.cc DH_Example.cc StopWatch.cc

TestBidirectional_SOURCES 	= TestBidirectional.cc DH_Example.cc

tTH_Socket_SOURCES 		= tTH_Socket.cc DH_Socket.cc

LDADD				= ../src/libtransport.la 
DEPENDENCIES			= ../src/libtransport.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  MPIBIN=$(MPIBIN)

TESTS	=					\
	Example_test.sh				\
	TestBidirectional_test.sh		\
	ExampleMem_test.sh			\
	ExampleBlMem_test.sh			\
	ExampleNonBlocking_test.sh		\
	ExampleShMem_test.sh			\
	ExampleMPI_test.sh			\
	ExampleVarBuf_test.sh			\
	ExampleSocket_test.sh		        \
	tTH_Socket_test.sh


include $(lofar_sharedir)/Makefile.common
