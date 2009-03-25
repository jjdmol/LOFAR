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
  Net/Socket.cc)

if(HAVE_BACKTRACE)
  list(APPEND common_LIB_SRCS 
    AddressTranslator.cc 
    Backtrace.cc
    SymbolTable.cc)
endif(HAVE_BACKTRACE)

if(HAVE_SHMEM)
  include(JoinArguments)
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

add_library(common ${common_LIB_SRCS})
target_link_libraries(common ${LOFAR_LIBRARIES})
install(TARGETS common DESTINATION ${LOFAR_LIBDIR})

#add_executable(versioncommon versioncommon.cc)
#target_link_libraries(versioncommon common)
#install(TARGETS versioncommon DESTINATION bin)

add_executable(getparsetvalue getparsetvalue.cc)
target_link_libraries(getparsetvalue common ${LOFAR_LIBRARIES})
install(TARGETS getparsetvalue DESTINATION bin)
