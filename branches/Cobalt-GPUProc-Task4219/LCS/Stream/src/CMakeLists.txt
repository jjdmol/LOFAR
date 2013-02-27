# $Id$

include(LofarPackageVersion)

lofar_add_library(stream
  Package__Version.cc
  FileDescriptorBasedStream.cc
  FileStream.cc
  FixedBufferStream.cc
  NamedPipeStream.cc
  NullStream.cc
  PortBroker.cc
  SharedMemoryStream.cc
  SocketStream.cc
  Stream.cc
  StringStream.cc)

lofar_add_bin_program(versionstream versionstream.cc)
