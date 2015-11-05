# $Id$

include(LofarPackageVersion)

lofar_add_library(transport
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
  TransportHolder.cc)

lofar_add_bin_program(versiontransport versiontransport.cc)

