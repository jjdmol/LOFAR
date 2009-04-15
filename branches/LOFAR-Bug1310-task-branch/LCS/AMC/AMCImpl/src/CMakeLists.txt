set(amcimpl_LIB_SRCS
  Package__Version.cc
  ConverterImpl.cc
  ConverterProcess.cc
  ConverterServer.cc
)

set(amcimpl_PROGRAMS
  versionamcimpl
  AMCServer
  WGS84toITRF)

add_library(amcimpl ${amcimpl_LIB_SRCS})
target_link_libraries(amcimpl common transport amcbase ${LOFAR_LIBRARIES})
install(TARGETS amcimpl DESTINATION ${LOFAR_LIBDIR})

foreach(prog ${amcimpl_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  target_link_libraries(${prog} amcimpl ${LOFAR_LIBRARIES})
  install(TARGETS ${prog} DESTINATION bin)
endforeach(prog ${amcimpl_PROGRAMS})
