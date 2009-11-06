# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  GCF_Control.h
  GCF_Scheduler.h
  GCF_DevicePort.h
  GCF_ETHRawPort.h
  GCF_Fsm.h
  GCF_Handler.h
  GCF_ITCPort.h
  GCF_Port.h
  GCF_PortInterface.h
  GCF_Protocols.h
  GCF_RawPort.h
  GCF_Task.h
  GCF_TCPPort.h
  GCF_TimerPort.h)

# Add definition of include path suffix definition, because the header files
# reside in a non-standard location. 
set(${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX GCF/TM
  CACHE INTERNAL "Include path suffix for package \"${PACKAGE_NAME}\"")

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/GCF/TM)

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/GCF/TM)
