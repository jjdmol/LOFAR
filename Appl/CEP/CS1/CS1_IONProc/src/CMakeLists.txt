
set (liblofar_impl_SOURCES
  BeamletBuffer.cc
  BGL_Personality.cc
  CS1_ION_main.cc
  Connector.cc
  InputThread.cc
  ION_Allocator.cc
  ReaderWriterSynchronization.cc
  InputSection.cc
  WH_DelayCompensation.cc
  AH_ION_Gather.cc
  WH_ION_Gather.cc
  TH_ZoidServer.cc
#  Package__Version.cc
  )

add_library (lofar_impl ${liblofar_impl_SOURCES})

