include(JoinArguments)

include_directories(${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../include)

########### next target ###############

message(STATUS "[LCS/Common/src] LOFAR_LIBRARIES = ${LOFAR_LIBRARIES}")

set(common_LIB_SRCS
   Allocator.cc
   DataConvert.cc
   Exception.cc
   FileLocator.cc
   hexdump.cc
   KVpair.cc
   lofar_bitset.cc
   LofarLogCout.cc
   LofarLogger.cc
   Numeric.cc
   ParameterSet.cc
   ParameterSetImpl.cc
   ParameterValue.cc
   PrettyUnits.cc
   Process.cc
   RunOnNode.cc
   Stopwatch.cc
   StreamUtil.cc
   StringUtil.cc
   SystemUtil.cc
   Timer.cc
   TypeNames.cc
   Net/FdSet.cc
   Net/Socket.cc
)

if(HAVE_SHMEM)
  set(shmem_LIB_SRCS
    shmem/segbasemap.cc
    shmem/dlmalloc.c
    shmem/shmem_alloc.cc)
  set(shmem_COMPILE_FLAGS
    -DUSE_PUBLIC_MALLOC_WRAPPERS
    -DUSE_DL_PREFIX
    -DHAVE_MMAP=0
    -DMORECORE=shmbrk
    -DMORECORE_CONTIGUOUS=0
    -DMORECORE_CANNOT_TRIM=1
    -DSHMEM_ALLOC
    -DHAVE_USR_INCLUDE_MALLOC_H)
  join_arguments(shmem_COMPILE_FLAGS)
  set_source_files_properties(${shmem_LIB_SRCS} 
    PROPERTIES COMPILE_FLAGS ${shmem_COMPILE_FLAGS})
  list(APPEND common_LIB_SRCS ${shmem_LIB_SRCS})
endif(HAVE_SHMEM)

add_library(common STATIC ${common_LIB_SRCS})
target_link_libraries(common ${LOFAR_LIBRARIES})
install(TARGETS common DESTINATION lib)


#add_executable(versioncommon versioncommon.cc)
#target_link_libraries(versioncommon common)
#install(TARGETS versioncommon DESTINATION bin)


add_executable(getparsetvalue getparsetvalue.cc)
target_link_libraries(getparsetvalue common ${LOFAR_LIBRARIES})
install(TARGETS getparsetvalue DESTINATION bin)


########### install files ###############




#original Makefile.am contents follow:

#if HAVE_SHMEM
#SUBDIRS	= shmem .
#endif
#
#if USE_BACKTRACE
#backtrace_SRCS = AddressTranslator.cc Backtrace.cc SymbolTable.cc
#endif
#
#lib_LTLIBRARIES 	= libcommon.la
#
#libcommon_la_SOURCES 	= Package__Version.cc \
#	Allocator.cc \
#	DataConvert.cc \
#	Exception.cc \
#	FileLocator.cc \
#	hexdump.cc \
#	KVpair.cc \
#	lofar_bitset.cc \
#	LofarLogCout.cc \
#	LofarLogger.cc \
#	Numeric.cc \
#	ParameterSet.cc \
#	ParameterSetImpl.cc \
#	ParameterValue.cc \
#	PrettyUnits.cc \
#	Process.cc \
#	RunOnNode.cc \
#	Stopwatch.cc \
#	StreamUtil.cc \
#	StringUtil.cc \
#	SystemUtil.cc \
#	Timer.cc \
#	TypeNames.cc \
#	Version.cc \
#	Net/FdSet.cc \
#	Net/Socket.cc
#
#libcommon_la_SOURCES	+= $(backtrace_SRCS)
#
#if HAVE_SHMEM
#libcommon_la_LIBADD	= shmem/libshmem.la
#endif
#
#bin_PROGRAMS = versioncommon getparsetvalue
#
#versioncommon_SOURCES      = versioncommon.cc
#versioncommon_LDADD        = libcommon.la
#versioncommon_DEPENDENCIES = libcommon.la $(LOFAR_DEPEND)
#
#getparsetvalue_SOURCES      = getparsetvalue.cc
#getparsetvalue_LDADD        = libcommon.la
#getparsetvalue_DEPENDENCIES = libcommon.la $(LOFAR_DEPEND)
#
#include $(top_srcdir)/Makefile.common
