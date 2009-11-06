# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  GCF_PValue.h
  GCF_Defines.h
  GCF_PVChar.h
  GCF_PVBlob.h
  GCF_PVBool.h
  GCF_PVDateTime.h
  GCF_PVDouble.h
  GCF_PVDynArr.h
  GCF_PVInteger.h
  GCF_PVString.h
  GCF_PVTypes.h
  GCF_PVUnsigned.h
  PVSSinfo.h
  PVSSresponse.h
  PVSSresult.h
  PVSSservice.h)

# Add definition of include path suffix definition, because the header files
# reside in a non-standard location. 
set(${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX GCF/PVSS
  CACHE INTERNAL "Include path suffix for package \"${PACKAGE_NAME}\"")

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/GCF/PVSS)

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/GCF/PVSS)
