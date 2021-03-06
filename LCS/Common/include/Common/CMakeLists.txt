# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES 
  Allocator.h
  Backtrace.h
  CasaLogSink.h
  CheckConfig.h
  compiler.h
  ComplexBuiltinFP.h
  ComplexBuiltinInt.h
  ComplexC99.h
  ComplexStdFP.h
  ComplexStdInt.h
  DataConvert.h
  DataFormat.h
  Exception.h
  Exceptions.h
  FileLocator.h
  hexdump.h
  InputParSet.h
  i4complex.h
  KVpair.h
  lofar_algorithm.h
  lofar_bitset.h
  LofarBitModeInfo.h
  LofarConstants.h
  lofar_complex.h
  lofar_deque.h
  lofar_fstream.h
  lofar_iomanip.h
  lofar_iosfwd.h
  lofar_iostream.h
  lofar_list.h
  LofarLocators.h
  LofarLog4Cplus.h
  LofarLog4Cxx.h
  LofarLogCout.h
  LofarLogger.h
  lofar_map.h
  lofar_math.h
  lofar_numeric.h
  lofar_set.h
  lofar_smartptr.h
  lofar_sstream.h
  lofar_stack.h
  lofar_string.h
  lofar_thread.h
  lofar_tribool.h
  lofar_typeinfo.h
  LofarTypedefs.h
  LofarTypes.h
  lofar_vector.h
  Mmap.h
  Numeric.h
  NsTimestamp.h
  ObjectFactory.h
  OpenMP.h
  ParameterRecord.h
  ParameterSet.h
  ParameterSetImpl.h
  ParameterValue.h
  PrettyUnits.h
  Process.h
  ReadLine.h
  RunOnNode.h
  Singleton.h
  Stopwatch.h
  StreamUtil.h
  StringUtil.h
  SystemCallException.h
  SystemUtil.h
  Timer.h
  TypeNames.h
  TypeNames.tcc
  Version.h
  DESTINATION include/${PACKAGE_NAME})

install(FILES 
  Net/FdSet.h
  Net/Socket.h
  DESTINATION include/${PACKAGE_NAME}/Net)

install(FILES
  shmem/segbasemap.h
  shmem/dlmalloc.h
  shmem/shmem_alloc.h
  DESTINATION include/${PACKAGE_NAME}/shmem)

install(FILES
  Thread/Cancellation.h
  Thread/Condition.h
  Thread/Mutex.h
  Thread/Queue.h
  Thread/Semaphore.h
  Thread/Thread.h
  DESTINATION include/${PACKAGE_NAME}/Thread)
