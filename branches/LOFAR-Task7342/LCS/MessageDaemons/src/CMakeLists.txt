# $Id$

include(LofarPackageVersion)

set(messagedaemons_LIB_SRCS
  Package__Version.cc)

lofar_add_library(messagedaemons ${messagedaemons_LIB_SRCS})

install(FILES
  MessageRouter
  QPIDWebserverJSON
  DESTINATION bin)

file(GLOB MessageRouterConfs
  MessageRouter.conf*
  )

file(GLOB WebMonitorFiles
  lib/*
  Schema.svg.*
  *.html
  )

install(FILES
  ${MessageRouterConfs}
  ${WebMonitorFiles}
  DESTINATION etc)
