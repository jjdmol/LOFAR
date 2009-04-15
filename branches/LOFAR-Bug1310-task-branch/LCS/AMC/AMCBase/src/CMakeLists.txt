set(amcbase_LIB_SRCS
  Package__Version.cc
  BlobIO.cc
  ConverterClient.cc
  ConverterCommand.cc
  ConverterStatus.cc
  Coord3D.cc
  DH_Request.cc
  DH_Result.cc
  Direction.cc
  Epoch.cc
  Position.cc)

set(amcbase_PROGRAMS
  versionamcbase
)

add_library(amcbase ${amcbase_LIB_SRCS})
target_link_libraries(amcbase common blob transport ${LOFAR_LIBRARIES})
install(TARGETS amcbase DESTINATION ${LOFAR_LIBDIR})

foreach(prog ${amcbase_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  target_link_libraries(${prog} amcbase ${LOFAR_LIBRARIES})
  install(TARGETS ${prog} DESTINATION bin)
endforeach(prog ${amcbase_PROGRAMS})
