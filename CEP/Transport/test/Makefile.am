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

check_PROGRAMS			= Example ExamplePL ExamplePL2

Example_SOURCES 		= Example.cc DH_Example.cc

ExamplePL_SOURCES 		= ExamplePL.cc DH_Example.cc

ExamplePL2_SOURCES 		= ExamplePL2.cc DH_Example2.cc

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
	ExamplePL2_test.sh			


include $(lofar_sharedir)/Makefile.common
