include(LofarCTest)

set(CHECK_PROGRAMS
  Example
  TestBidirectional
  ExampleMem
  ExampleBlMem
  ExampleNonBlocking
  ExampleVarBuf
  ExampleSocket
#  ExampleEthernet
#  tServer tClient
#  tCSConnection
)

if(HAVE_MPI)
  list(APPEND CHECK_PROGRAMS ExampleMPI)
  add_executable(ExampleMPI ExampleMPI.cc DH_Example.cc DH_ExampleExtra.cc)
  if(HAVE_SHMEM)
    set(shmem_COMPILE_FLAGS
      -DHAVE_MMAP=0
      -DMORECORE=shmbrk
      -DMORECORE_CONTIGUOUS=0
      -DMORECORE_CANNOT_TRIM=1
      -DSHMEM_ALLOC)
    join_arguments(shmem_COMPILE_FLAGS)
    set_source_files_properties(ExampleShMem.cc 
      PROPERTIES COMPILE_FLAGS ${shmem_COMPILE_FLAGS})
    list(APPEND CHECK_PROGRAMS ExampleShMem)
    add_executable(ExampleShMem ExampleShMem.cc DH_Example.cc DH_ExampleExtra.cc)
  endif(HAVE_SHMEM)
endif(HAVE_MPI)

add_executable(Example Example.cc DH_Example.cc)
add_executable(ExampleMem ExampleMem.cc DH_Example.cc DH_ExampleExtra.cc)
add_executable(ExampleBlMem ExampleBlMem.cc DH_Example.cc DH_ExampleExtra.cc)
add_executable(ExampleNonBlocking ExampleNonBlocking.cc DH_Example.cc)
add_executable(ExampleVarBuf ExampleVarBuf.cc DH_VarBuf.cc)
add_executable(TestBidirectional TestBidirectional.cc DH_Example.cc)
add_executable(ExampleSocket ExampleSocket.cc DH_Socket.cc)
#add_executable(ExampleEthernet ExampleEthernet.cc DH_Ethernet.cc)
#add_executable(tServer tServer.cc DH_Socket.cc)
#add_executable(tClient tClient.cc DH_Socket.cc)
#add_executable(tCSConnection tCSConnection.cc)

foreach (prog ${CHECK_PROGRAMS})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}_test.sh)
  add_dependencies(check ${prog})
  target_link_libraries(${prog} transport ${LOFAR_LIBRARIES})
endforeach (prog CHECK_PROGRAMS)
