set(common_LIB_SRCS
  Package__Version.cc
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
  Version.cc
  Net/FdSet.cc
  Net/Socket.cc)

set(common_PROGRAMS
  getparsetvalue
  versioncommon
)

if(HAVE_BACKTRACE)
  list(APPEND common_LIB_SRCS 
    AddressTranslator.cc 
    Backtrace.cc
    SymbolTable.cc)
endif(HAVE_BACKTRACE)

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

add_library(common ${common_LIB_SRCS})
install(TARGETS common DESTINATION ${LOFAR_LIBDIR})

foreach(prog ${common_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  target_link_libraries(${prog} common ${LOFAR_LIBRARIES})
  install(TARGETS ${prog} DESTINATION bin)
endforeach(prog ${common_PROGRAMS})
