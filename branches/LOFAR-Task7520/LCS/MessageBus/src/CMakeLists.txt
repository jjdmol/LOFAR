# $Id$

include(LofarPackageVersion)
include(PythonInstall)

set(messagebus_LIB_SRCS
  Package__Version.cc
  LogSink.cc
  MsgBus.cc
  Message.cc)

set(messagebus_PROGRAMS
)

lofar_add_library(messagebus ${messagebus_LIB_SRCS})

foreach(prog ${messagebus_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${messagebus_PROGRAMS})

python_install(
  __init__.py
  msgbus.py
  basemsgbus.py
  message.py
  DESTINATION lofar/messagebus)

add_subdirectory(protocols)
