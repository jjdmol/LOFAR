# $Id$

include(LofarPackageVersion)

set(messagedaemons_LIB_SRCS
  Package__Version.cc)

lofar_add_library(messagedaemons ${messagedaemons_LIB_SRCS})

install(FILES
  MessageRouter
  DESTINATION bin)

file(GLOB MessageRouterConfs
  MessageRouter.conf*
  )

install(FILES
  ${MessageRouterConfs}
  DESTINATION etc)
