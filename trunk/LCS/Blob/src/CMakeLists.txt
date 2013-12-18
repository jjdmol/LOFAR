# $Id$

include(LofarPackageVersion)

# Process input files for Flex and Bison
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(KeyParse KeyParse.yy 
  ${CMAKE_CURRENT_BINARY_DIR}/KeyParse.cc
  COMPILE_FLAGS "-y -pKeyParse")

flex_target(KeyTokenize KeyTokenize.ll 
  ${CMAKE_CURRENT_BINARY_DIR}/KeyTokenize.cc
  COMPILE_FLAGS "-PKeyTokenize")

add_flex_bison_dependency(KeyTokenize KeyParse)

# Create symbolic link from KeyParse.h -> KeyParse.hh, because KeyParser.cc
# #include's KeyParse.h.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  KeyParse.hh
  KeyParse.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# On Apple, bison creates KeyParse.cc.h, instead of KeyParse.hh. So, when on
# Apple, we need to create another symbolic link KeyParse.hh -> KeyParse.cc.h
if(APPLE)
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
    KeyParse.cc.h
    KeyParse.hh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif(APPLE)

# Add current binary directory to include, because that's where flex/bison
# will output the files they generate.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

lofar_add_library(blob
  Package__Version.cc
  BlobAipsIO.cc
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
  ${BISON_KeyParse_OUTPUTS}
  ${FLEX_KeyTokenize_OUTPUTS}
  KeyParser.cc
  KeyValue.cc
  KeyValueMap.cc)

lofar_add_bin_program(versionblob versionblob.cc)

