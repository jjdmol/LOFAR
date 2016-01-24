# $Id: CMakeLists.txt 1457 2015-08-18 14:44:52Z loose $

lofar_add_library(messaging
  LofarMessage.cc
  DefaultSettings.cc
  EventMessage.cc
  FromBus.cc
  Helpers.cc
  Message.cc
  MonitoringMessage.cc
  ProgressMessage.cc
  ServiceMessage.cc
  ToBus.cc)
