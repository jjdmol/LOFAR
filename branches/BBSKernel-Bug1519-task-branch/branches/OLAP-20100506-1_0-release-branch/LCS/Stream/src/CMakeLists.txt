# $Id$

include(LofarPackageVersion)

lofar_add_library(stream
  Package__Version.cc
  FileDescriptorBasedStream.cc
  FileStream.cc
  NullStream.cc
  SocketStream.cc
  Stream.cc)

lofar_add_bin_program(versionstream versionstream.cc)
