include(LofarCTest)

lofar_add_test(Example Example.cc DH_Example.cc)
lofar_add_test(ExampleMem ExampleMem.cc DH_Example.cc DH_ExampleExtra.cc)
lofar_add_test(ExampleBlMem ExampleBlMem.cc DH_Example.cc DH_ExampleExtra.cc)
lofar_add_test(ExampleNonBlocking ExampleNonBlocking.cc DH_Example.cc)
lofar_add_test(ExampleVarBuf ExampleVarBuf.cc DH_VarBuf.cc)
lofar_add_test(TestBidirectional TestBidirectional.cc DH_Example.cc)
lofar_add_test(ExampleSocket ExampleSocket.cc DH_Socket.cc)
#lofar_add_test(ExampleEthernet ExampleEthernet.cc DH_Ethernet.cc)
#lofar_add_test(tServer tServer.cc DH_Socket.cc)
#lofar_add_test(tClient tClient.cc DH_Socket.cc)
#lofar_add_test(tCSConnection tCSConnection.cc)

if(HAVE_MPI)
  lofar_add_test(ExampleMPI ExampleMPI.cc DH_Example.cc DH_ExampleExtra.cc)
  if(HAVE_SHMEM)
    set(shmem_COMPILE_FLAGS
      -DHAVE_MMAP=0
      -DMORECORE=shmbrk
      -DMORECORE_CONTIGUOUS=0
      -DMORECORE_CANNOT_TRIM=1
      -DSHMEM_ALLOC)
    lofar_join_arguments(shmem_COMPILE_FLAGS)
    set_source_files_properties(ExampleShMem.cc 
      PROPERTIES COMPILE_FLAGS ${shmem_COMPILE_FLAGS})
    lofar_add_test(ExampleShMem ExampleShMem.cc DH_Example.cc DH_ExampleExtra.cc)
  endif(HAVE_SHMEM)
endif(HAVE_MPI)
