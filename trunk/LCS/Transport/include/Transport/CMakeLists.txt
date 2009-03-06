## ------------------------------------------------------------------------------
## Include files

file (GLOB libtransport_HEADERS BaseSim.h
				BGLConnection.h
				Connection.h
				ConnectionPool.h
				CSConnection.h
				DataBlobExtra.h
				DataHolder.h
				DH_BlobStreamable.h
				TH_Ethernet.h
				TH_File.h
				TH_Mem.h
				TH_MPI.h
				TH_Null.h
				TH_ShMem.h
				TH_Socket.h
				TransportHolder.h )

## ------------------------------------------------------------------------------
## Installation
   
install (FILES ${libtransport_HEADERS}
  DESTINATION include/Transport
  )
