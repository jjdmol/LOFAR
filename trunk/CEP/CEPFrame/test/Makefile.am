common_top_builddir=@common_top_builddir@

CFLAGS += \
	-DUSE_PUBLIC_MALLOC_WRAPPERS \
	-DUSE_DL_PREFIX \
	-DHAVE_MMAP=0 \
	-DMORECORE=shmbrk \
	-DMORECORE_CONTIGUOUS=0 \
	-DMORECORE_CANNOT_TRIM=1 \
	-DSHMEM_ALLOC

CXXFLAGS += \
	-DUSE_PUBLIC_MALLOC_WRAPPERS \
	-DUSE_DL_PREFIX \
	-DHAVE_MMAP=0 \
	-DMORECORE=shmbrk \
	-DMORECORE_CONTIGUOUS=0 \
	-DMORECORE_CANNOT_TRIM=1 \
	-DSHMEM_ALLOC

check_PROGRAMS			= Example 		\
				  Simulator_Example	\
				  Tester		\
				  shmem_alloc1		\
				  shmem_alloc2		\
				  TestAutoTrigger       \
				  CyclicBufferTest	\
				  tmat	                

Example_SOURCES 		= Example.cc
Simulator_Example_SOURCES	= Simulator_Example.cc Simulator_Example.h
Tester_SOURCES			= Tester.cc
tmat_SOURCES			= tmat.cc
shmem_alloc1_SOURCES		= shmem_alloc1.cc
shmem_alloc2_SOURCES		= shmem_alloc2.cc
TestAutoTrigger_SOURCES         = TestAutoTrigger.cc
CyclicBufferTest_SOURCES	= CyclicBufferTest.cc


LDADD				= ../src/libcepframe.la
DEPENDENCIES			= ../src/libcepframe.la $(LOFAR_DEPEND)

CyclicBufferTest_LDADD		= ../src/libcepframe.la -lpthread
CyclicBufferTest_DEPENDENCIES	= ../src/libcepframe.la $(LOFAR_DEPEND) -lpthread

#Example_LDADD 			= ../src/libcepframe.la
#Example_DEPENDENCIES		= ../src/libcepframe.la $(LOFAR_DEPEND)
#Simulator_Example_LDADD	= ../src/libcepframe.la
#Simulator_Example_DEPENDENCIES	= ../src/libcepframe.la $(LOFAR_DEPEND)
#Tester_LDADD			= ../src/libcepframe.la
#Tester_DEPENDENCIES		= ../src/libcepframe.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir) \
                                  MPIBIN=$(MPIBIN)

TESTS	=					\
	Example_test.sh				\
	Simulator_Example_test.sh		\
	Tester_test.sh				\
	tmat_test.sh				\
	shmem_alloc1.sh				\
	shmem_alloc2.sh				\
	TestAutoTrigger.sh			\
	CyclicBufferTest.sh

EXTRA_DIST = 					\
	Example_test.sh				\
	Example.out				\
	Simulator_Example.in			\
	Simulator_Example.out			\
	Simulator_Example_test.sh		\
	Tester.out				\
	Tester_test.sh				\
	tmat_test.sh				\
	shmem_alloc1.sh				\
	shmem_alloc2.sh				\
	TestAutoTrigger.sh			\
	CyclicBufferTest.sh			\
	runtest.sh				\
	cleanipc				\
	checkipc


include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
