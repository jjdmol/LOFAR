## ------------------------------------------------------------------------------
## Build the library

file (GLOB libtransport_SOURCES BGLConnection.cc
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
				TransportHolder.cc)

add_library (transport ${libtransport_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS transport
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

