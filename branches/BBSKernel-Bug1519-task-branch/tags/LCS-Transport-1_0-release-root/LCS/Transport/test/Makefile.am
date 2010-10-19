common_top_builddir=@common_top_builddir@

CFLAGS += \
	-DHAVE_MMAP=0 \
	-DMORECORE=shmbrk \
	-DMORECORE_CONTIGUOUS=0 \
	-DMORECORE_CANNOT_TRIM=1 \
	-DSHMEM_ALLOC

CXXFLAGS += \
	-DHAVE_MMAP=0 \
	-DMORECORE=shmbrk \
	-DMORECORE_CONTIGUOUS=0 \
	-DMORECORE_CANNOT_TRIM=1 \
	-DSHMEM_ALLOC

check_PROGRAMS			= Example TestBidirectional ExampleMem ExampleBlMem  ExampleShMem ExampleMPI ExampleSocket ExampleSocketPerf tTH_Socket

Example_SOURCES 		= Example.cc DH_Example.cc

ExampleMem_SOURCES 		= ExampleMem.cc DH_Example.cc

ExampleBlMem_SOURCES		= ExampleBlMem.cc DH_Example.cc

ExampleShMem_SOURCES 		= ExampleShMem.cc DH_Example.cc

ExampleMPI_SOURCES 		= ExampleMPI.cc DH_Example.cc

ExampleSocket_SOURCES		= ExampleSocket.cc DH_Example.cc

ExampleSocketPerf_SOURCES	= ExampleSocketPerf.cc DH_Example.cc StopWatch.cc

TestBidirectional_SOURCES 	= TestBidirectional.cc DH_Example.cc

tTH_Socket_SOURCES 			= tTH_Socket.cc DH_Socket.cc
tTH_Socket_LDADD 			= ../src/libtransport.la
tTH_Socket_DEPENDENCIES 	= ../src/libtransport.la $(LOFAR_DEPEND)

LDADD				= ../src/libtransport.la 
DEPENDENCIES			= ../src/libtransport.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir) \
				  MPIBIN=$(MPIBIN)

TESTS	=					\
	Example_test.sh				\
	TestBidirectional_test.sh		\
	ExampleMem_test.sh			\
	ExampleBlMem_test.sh			\
	ExampleShMem_test.sh			\
	ExampleMPI_test.sh			\
	ExampleSocket_test.sh		\
	tTH_Socket_test.sh


include $(lofar_sharedir)/Makefile.common
