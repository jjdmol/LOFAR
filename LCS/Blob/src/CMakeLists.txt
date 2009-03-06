#make_package_version (${LCS_SOURCE_DIR}/Blob/src TRUE TRUE)

## ------------------------------------------------------------------------------
## Process input files for Flex and Bison

if (bison_bin)
  execute_process (
    WORKING_DIRECTORY ${LCS_SOURCE_DIR}/Blob/src
    COMMAND bison -y -d -p KeyParse KeyParse.yy -o KeyParse
    TIMEOUT 20
    ERROR_VARIABLE yacc_error
    OUTPUT_QUIET
    )
  execute_process (
    WORKING_DIRECTORY ${LCS_SOURCE_DIR}/Blob/src
    COMMAND mv KeyParse KeyParse.cc
    )
else (bison_bin)
  message (SEND_ERROR "Unable to run YACC; missing application!")
endif (bison_bin)

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

##file (GLOB libblob_SOURCES *.cc Net/*.cc)
file (GLOB libblob_SOURCES BlobAipsIO.cc
        		   BlobArray.cc
        		   BlobField2.cc
        		   BlobFieldSet.cc
        		   BlobHeader.cc
        		   BlobIBufChar.cc
        		   BlobIBufStream.cc
        		   BlobIStream.cc
        		   BlobOBufChar.cc
        		   BlobOBufNull.cc
        		   BlobOBufStream.cc
        		   BlobOBufString.cc
        		   BlobOStream.cc
			   BlobStreamable.cc
        		   BlobString.cc
        		   BlobStringType.cc
        		   KeyParse.cc
        		   KeyTokenize.cc
        		   KeyParser.cc
        		   KeyValue.cc
        		   KeyValueMap.cc )

add_library (blob ${libblob_SOURCES})

target_link_libraries (blob ${CASA_LIBRARIES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS blob
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

