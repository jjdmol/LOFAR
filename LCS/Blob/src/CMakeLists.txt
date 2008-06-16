
make_package_version (${LCS_SOURCE_DIR}/Blob/src TRUE TRUE)

## ------------------------------------------------------------------------------
## Process input files for Flex and Bison

if (yacc_bin)
  execute_process (
    WORKING_DIRECTORY ${LCS_SOURCE_DIR}/Blob/src
    COMMAND yacc -p KeyParse -o KeyParse.h KeyParse.yy
    TIMEOUT 20
    ERROR_VARIABLE yacc_error
    OUTPUT_QUIET
    )
endif (yacc_bin)

## ------------------------------------------------------------------------------
## Build the library

file (GLOB libblob_SOURCES *.cc Net/*.cc)

add_library (blob ${libblob_SOURCES})

target_link_libraries (blob ${CASA_LIBRARIES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS blob
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

