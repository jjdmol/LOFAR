# $Id$

# List of header files that will be installed.
set (inst_HEADERS
  BlobAipsIO.h
  BlobArray.h
  BlobArray.tcc
  BlobArrayHeader.h
  BlobException.h
  BlobField.h
  BlobField.tcc
  BlobFieldSet.h
  BlobHeader.h
  BlobIBufChar.h
  BlobIBuffer.h
  BlobIBufStream.h
  BlobIBufString.h
  BlobIBufVector.h
  BlobIStream.h
  BlobOBufChar.h
  BlobOBuffer.h
  BlobOBufNull.h
  BlobOBufStream.h
  BlobOBufString.h
  BlobOBufVector.h
  BlobOStream.h
  BlobSTL.h
  BlobSTL.tcc
  BlobStreamable.h
  BlobString.h
  BlobStringTraits.h
  BlobStringType.h
  KeyParser.h
  KeyValue.h
  KeyValueMap.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
