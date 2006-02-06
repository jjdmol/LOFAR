AM_CPPFLAGS = \
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
				  tmat	                \
				  CyclicBufferTest

Example_SOURCES 		= Example.cc
Simulator_Example_SOURCES	= Simulator_Example.cc Simulator_Example.h
Tester_SOURCES			= Tester.cc
tmat_SOURCES			= tmat.cc
shmem_alloc1_SOURCES		= shmem_alloc1.cc
shmem_alloc2_SOURCES		= shmem_alloc2.cc
TestAutoTrigger_SOURCES         = TestAutoTrigger.cc TestAutoTrigger.h \
				  WH_TestAutoTrigger.cc WH_TestAutoTrigger.h
CyclicBufferTest_SOURCES	= CyclicBufferTest.cc

LDADD				= ../src/libcepframe.la
DEPENDENCIES			= ../src/libcepframe.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  top_srcdir=$(top_srcdir) \
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

EXTRA_DIST = $(TESTS)				\
	CyclicBufferTest.gnuplot		\
	CyclicBufferTest.run			\
	Example.run				\
	Example.stdout				\
	Simulator_Example.in			\
	Simulator_Example.run			\
	Simulator_Example.stdout		\
	TestAutoTrigger.run			\
	TestAutoTrigger.stdout			\
	Tester.run				\
	Tester.stdout				\
	cleanipc				\
	checkipc

clean-local:
	$(RM) CB.* gnuplot.cmd

include $(top_srcdir)/Makefile.common
