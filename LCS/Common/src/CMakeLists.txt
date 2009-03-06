#make_package_version (${LCS_SOURCE_DIR}/Common/src TRUE TRUE)

## ------------------------------------------------------------------------------
## Build the library

if (Backtrace_FOUND)
  file (GLOB backtrace_SRCS AddressTranslator.cc
                            Backtrace.cc
			    SymbolTable.cc )
endif (Backtrace_FOUND)

if (HAVE_SHEM)
  file (GLOB shmem_SRCS     shmem/dlmalloc.c
                            shmem/segbasemap.cc
                            shmem/shmem_alloc.cc )
  add_library (shmem ${shmem_SRCS})
  
  set_target_properties( shmem PROPERTIES
                         COMPILE_FLAGS "-DUSE_PUBLIC_MALLOC_WRAPPERS -DUSE_DL_PREFIX -DHAVE_MMAP=0 -DMORECORE=shmbrk -DMORECORE_CONTIGUOUS=0 -DMORECORE_CANNOT_TRIM=1 -DSHMEM_ALLOC -DHAVE_USR_INCLUDE_MALLOC_H -DUSE_THREADS -pthread"
  )
  
  
endif (HAVE_SHEM)

file (GLOB libcommon_SOURCES Allocator.cc
			     DataConvert.cc
			     Exception.cc
			     FileLocator.cc
			     hexdump.cc
			     KVpair.cc
			     Lock.cc
			     lofar_bitset.cc
			     LofarLogCout.cc
			     LofarLogger.cc
			     Numeric.cc
			     ParameterSet.cc
			     ParameterSetImpl.cc
			     ParameterValue.cc
			     PrettyUnits.cc
			     Process.cc
			     ReadLine.cc
			     RunOnNode.cc
			     Stopwatch.cc
			     StreamUtil.cc
			     StringUtil.cc
			     SystemUtil.cc
			     Timer.cc
			     TypeNames.cc
			     Net/FdSet.cc
			     Net/Socket.cc
			     ${backtrace_SRCS})

add_library (common ${libcommon_SOURCES})

if (HAVE_SHEM)
target_link_libraries (common
                       shmem
                      )
endif (HAVE_SHEM)


## ------------------------------------------------------------------------------
## Installation
if (HAVE_SHEM)
install (
  TARGETS common shmem
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

else (HAVE_SHEM)
install (
  TARGETS common
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
endif (HAVE_SHEM)
