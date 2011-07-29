# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  ClusterDesc.h
  ControllerBase.h
  Controller.h
  DomainShape.h
  MPIConnection.h
  MPIConnectionSet.h
  MWBlobIO.h
  MWConnection.h
  MWConnectionSet.h
  MWError.h
  MWGlobalStep.h
  MWIos.h
  MWLocalStep.h
  MWMultiStep.h
  MWStep.h
  MWStepFactory.h
  MWStepVisitor.h
  MasterControl.h
  MemConnection.h
  MemConnectionSet.h
  NodeDesc.h
  ObsDomain.h
  ParameterHandler.h
  SocketConnection.h
  SocketConnectionSet.h
  SocketListener.h
  VdsDesc.h
  VdsPartDesc.h
  WorkDomainSpec.h
  WorkerControl.h
  WorkerFactory.h
  WorkerInfo.h
  WorkerProxy.h
  WorkersDesc.h) 

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
