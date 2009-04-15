set(transport_LIB_SRCS
  Package__Version.cc
  BGLConnection.cc
  Connection.cc
  ConnectionPool.cc
  CSConnection.cc
  DataBlobExtra.cc
  DataHolder.cc
  DH_BlobStreamable.cc
  TH_Ethernet.cc
  TH_File.cc
  TH_Mem.cc
  TH_MPI.cc
  TH_ShMem.cc
  TH_Socket.cc
  TransportHolder.cc
)

set(transport_PROGRAMS
  versiontransport
)

add_library(transport ${transport_LIB_SRCS})
target_link_libraries(transport blob)
install(TARGETS transport DESTINATION ${LOFAR_LIBDIR})

foreach(prog ${transport_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  target_link_libraries(${prog} transport ${LOFAR_LIBRARIES})
  install(TARGETS ${prog} DESTINATION bin)
endforeach(prog ${transport_PROGRAMS})

