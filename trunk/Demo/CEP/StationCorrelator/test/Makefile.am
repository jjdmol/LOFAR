AM_CPPFLAGS = \
	-DHAVE_MMAP=0 \
	-DMORECORE=shmbrk \
	-DMORECORE_CONTIGUOUS=0 \
	-DMORECORE_CANNOT_TRIM=1 \
	-DSHMEM_ALLOC

check_PROGRAMS			= CyclicBufferTest

CyclicBufferTest_SOURCES 	= CyclicBufferTest.cc


#LDADD				= ../src/libtransport.la 
#DEPENDENCIES			= ../src/libtransport.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  MPIBIN=$(MPIBIN)

TESTS	=					\
	CyclicBufferTest_test.sh		


include $(top_srcdir)/Makefile.common
