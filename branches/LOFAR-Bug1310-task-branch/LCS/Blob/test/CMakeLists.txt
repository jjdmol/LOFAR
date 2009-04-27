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
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}_test.sh)
  add_dependencies(check ${prog})
  target_link_libraries(${prog} blob)
endforeach(prog ${CHECK_PROGRAMS})
