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

check_PROGRAMS			= Example ExamplePL ExamplePL2 ExampleShMem ExampleSocket ExampleSocketPerf


Example_SOURCES 		= Example.cc DH_Example.cc

ExamplePL_SOURCES 		= ExamplePL.cc DH_Example.cc

ExamplePL2_SOURCES 		= ExamplePL2.cc DH_Example2.cc

ExampleShMem_SOURCES 		= ExampleShMem.cc DH_Example.cc

ExampleSocket_SOURCES		= ExampleSocket.cc DH_Example.cc

ExampleSocketPerf_SOURCES	= ExampleSocketPerf.cc DH_Example.cc StopWatch.cc

LDADD				= ../src/libtransport.la 
DEPENDENCIES			= ../src/libtransport.la $(LOFAR_DEPEND)

#Example_LDADD 			= ../src/libcepframe.la
#Example_DEPENDENCIES		= ../src/libcepframe.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

TESTS	=					\
	Example_test.sh				\
	ExamplePL_test.sh			\
	ExamplePL2_test.sh			\
	ExampleShMem_test.sh			\
	ExampleSocket_test.sh


include $(lofar_sharedir)/Makefile.common
