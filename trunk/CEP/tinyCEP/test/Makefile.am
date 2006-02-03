AM_CPPFLAGS		= \
        -DUSE_PUBLIC_MALLOC_WRAPPERS \
        -DUSE_DL_PREFIX \
        -DHAVE_MMAP=0 \
        -DMORECORE=shmbrk \
        -DMORECORE_CONTIGUOUS=0 \
        -DMORECORE_CANNOT_TRIM=1 \
        -DSHMEM_ALLOC \
        -DHAVE_USR_INCLUDE_MALLOC_H

check_PROGRAMS		= Example Example2 Example3


Example_SOURCES 	= DH_Example.h DH_Example.cc \
			  WH_Example.h WH_Example.cc \
			  MyExample.h MyExample.cc \
			  Example.cc 

Example2_SOURCES        = DH_Example.h DH_Example.cc \
			  WH_Example.h WH_Example.cc \
			  MySecondExample.h MySecondExample.cc \
			  Example2.cc

Example3_SOURCES	= DH_Example.h DH_Example.cc \
			  WH_Example.h WH_Example.cc \
			  MySocketExample.h MySocketExample.cc \
			  Example3.cc
EXTRA_Example3_DIST	= Example3.run Example3.stdout 

LDADD			= ../src/libtinycep.la
DEPENDENCIES		= ../src/libtinycep.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT	= top_srcdir=$(top_srcdir) \
			  lofar_sharedir=$(lofar_sharedir)

TESTS			= Example_test.sh  \
			  Example2_test.sh \
			  Example3_test.sh

EXTRA_DIST		= $(TESTS) \
			  $(EXTRA_Example3_DIST)

include $(top_srcdir)/Makefile.common
