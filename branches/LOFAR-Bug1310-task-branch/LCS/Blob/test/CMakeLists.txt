include(LofarCTest)

set(CHECK_PROGRAMS
    tBlobString
    tBlobStream
    tBlobArray
    tBlobSTL
    tBlobField
    tKeyValueMap
)

if(HAVE_AIPSPP)
  list(APPEND CHECK_PROGRAMS
    tBlobCasa
    tBlobAipsIO)
endif(HAVE_AIPSPP)

foreach(prog ${CHECK_PROGRAMS})
  add_executable(${prog} ${prog}.cc)
  target_link_libraries(${prog} blob ${LOFAR_LIBRARIES})
#  target_link_libraries(${prog} blob common ${LOFAR_LIBRARIES})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}_test.sh)
endforeach(prog ${CHECK_PROGRAMS})
