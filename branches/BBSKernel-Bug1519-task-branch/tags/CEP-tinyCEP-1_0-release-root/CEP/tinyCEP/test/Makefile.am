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

check_PROGRAMS			= Example Example2 Example3


Example_SOURCES 		= \
				DH_Example.cc \
				WH_Example.cc \
				MyExample.cc \
				Example.cc 

Example2_SOURCES           	= \
                                DH_Example.cc \
                                WH_Example.cc \
                                MySecondExample.cc \
                                Example2.cc

Example3_SOURCES		= \
				DH_Example.cc \
				WH_Example.cc \
				MySocketExample.cc \
				Example3.cc

LDADD				= ../src/libtinycep.la
DEPENDENCIES			= ../src/libtinycep.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

TESTS	=					\
	Example_test.sh				\
	Example2_test.sh			\
	Example3_test.sh

include $(lofar_sharedir)/Makefile.common
