cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/LCS/Blob/test )

## ------------------------------------------------------------------------------
## Sourses

file (GLOB tBlobString_SRCS  tBlobString.cc )
file (GLOB tBlobStream_SRCS  tBlobStream.cc )
file (GLOB tBlobArray_SRCS   tBlobArray.cc )
file (GLOB tBlobSTL_SRCS     tBlobSTL.cc )
file (GLOB tBlobCasa_SRCS    tBlobCasa.cc )
file (GLOB tBlobField_SRCS   tBlobField.cc )
file (GLOB tBlobAipsIO_SRCS  tBlobAipsIO.cc )
file (GLOB tKeyValueMap_SRCS tKeyValueMap.cc )

## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_BLOB_TEST _PROG)
  target_link_libraries (${_PROG} 
			 blob
			 common
			 ${CASA_LIBRARIES}
			 ${LOG4CPLUS_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 ${SHMEM_LIBRARIES} )
ENDMACRO (TARGETLINKLIBRARIES_BLOB_TEST)


## --- Programs to be built -------------------------

set (CHECKPROGS
     tBlobString
     tBlobStream
     tBlobArray
     tBlobSTL
     tBlobCasa
     tBlobField
     tBlobAipsIO
     tKeyValueMap
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_BLOB_TEST (${prog})
endforeach (prog ${CHECKPROGS})

## --- Programs to be run ---------------------------

set (CHECKTOOLPROGS
     tBlobString
     tBlobStream
     tBlobArray
     tBlobSTL
     tBlobCasa
     tBlobField
     tBlobAipsIO
     tKeyValueMap
)

foreach (prog ${CHECKTOOLPROGS})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}_test.sh) 
  set (TEST_PROGRAMS "${TEST_PROGRAMS};${prog}" CACHE STRING "Test programs" FORCE)
endforeach (prog ${CHECKTOOLPROGS})
















