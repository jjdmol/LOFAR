#
# automake specification for TH_ShMem transport.
#

# Definition of target library
lib_LTLIBRARIES 	= libth_shmem.la


CFLAGS += \
	-DUSE_PUBLIC_MALLOC_WRAPPERS \
	-DUSE_DL_PREFIX

CXXFLAGS += \
	-DUSE_PUBLIC_MALLOC_WRAPPERS \
	-DUSE_DL_PREFIX

DOCHDRS			= 			\
TH_ShMem.h					

libth_shmem_la_SOURCES  = 			\
$(DOCHDRS)					\
TH_ShMem.cc					

include $(lofar_sharedir)/Makefile.common
