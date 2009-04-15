set(plc_LIB_SRCS
  Package__Version.cc
  DH_ProcControl.cc
  PCCmd.cc
  ProcControlComm.cc
  ProcControlServer.cc
  ProcCtrlProxy.cc
  ProcCtrlCmdLine.cc
  ProcCtrlRemote.cc
  ACCmain.cc)

set(plc_PROGRAMS
  versionplc)

add_library(plc ${plc_LIB_SRCS})
target_link_libraries(plc common blob transport ${LOFAR_LIBRARIES})
install(TARGETS plc DESTINATION ${LOFAR_LIBDIR})

foreach(prog ${plc_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  target_link_libraries(${prog} plc ${LOFAR_LIBRARIES})
  install(TARGETS ${prog} DESTINATION bin)
endforeach(prog ${plc_PROGRAMS})
