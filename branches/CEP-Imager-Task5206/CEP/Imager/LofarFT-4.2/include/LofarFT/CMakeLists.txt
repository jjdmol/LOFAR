# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  FFTCMatrix.h
  VisResampler.h
  FTMachine.h
  FTMachineSimple.h
  Imager.h
  ATerm.h
  WTerm.h
  CFStore.h
  CFDefs.h
  VBStore.h
  VisBuffer.h
  VisibilityIterator.h
  VisImagingWeight.h
  Operation.h
  OperationClean.h
  OperationPredict.h
)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})

