set(alc_LIB_SRCS
  Package__Version.cc
  ACCmd.cc
  DH_ApplControl.cc
  ApplControlComm.cc
  ApplControlClient.cc
  ACSyncClient.cc
  ACAsyncClient.cc
  ApplControlServer.cc)

set(alc_PROGRAMS
  versionalc
)

add_library(alc ${alc_LIB_SRCS})
target_link_libraries(alc common blob transport ${LOFAR_LIBRARIES})
install(TARGETS alc DESTINATION ${LOFAR_LIBDIR})

foreach(prog ${alc_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  target_link_libraries(${prog} alc ${LOFAR_LIBRARIES})
  install(TARGETS ${prog} DESTINATION bin)
endforeach(prog ${alc_PROGRAMS})
