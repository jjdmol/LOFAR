
#make_package_version (${LCS_SOURCE_DIR}/Blob/src TRUE TRUE)

## ------------------------------------------------------------------------------
## Process input files for Flex and Bison

if (yacc_bin)
  execute_process (
    WORKING_DIRECTORY ${LCS_SOURCE_DIR}/Blob/src
    COMMAND yacc -d -p KeyParse -o KeyParse.h KeyParse.yy
    TIMEOUT 20
    ERROR_VARIABLE yacc_error
    OUTPUT_QUIET
    )
else (yacc_bin)
  message (SEND_ERROR "Unable to run YACC; missing application!")
endif (yacc_bin)

if (flex_bin)
   execute_process (
    WORKING_DIRECTORY ${LCS_SOURCE_DIR}/Blob/src
    COMMAND flex -t -PKeyTokenize KeyTokenize.ll
    TIMEOUT 20
    OUTPUT_FILE KeyTokenize.cc
    ERROR_VARIABLE lex_error
    OUTPUT_QUIET
   )
else (flex_bin)
  message (SEND_ERROR "Unable to run FLEX; missing application!")
endif (flex_bin)

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

