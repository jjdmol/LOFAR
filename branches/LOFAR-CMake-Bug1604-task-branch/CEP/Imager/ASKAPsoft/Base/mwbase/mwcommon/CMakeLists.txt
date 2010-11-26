# $Id$

lofar_package(scimath DEPENDS Common Blob)

lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS casa)
lofar_find_package(GSL REQUIRED)

# Because the ASKAP sources do not include lofar_config.h, we have to set the
# necessary preprocessor variables here manually.
if(HAVE_AIPSPP)
  add_definitions(-DHAVE_AIPSPP)
endif(HAVE_AIPSPP)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_library(askap_mwcommon
  src/ClusterDesc.cc
  src/DomainShape.cc
  src/MasterControl.cc
  src/MemConnection.cc
  src/MemConnectionSet.cc
  src/MPIConnection.cc
  src/MPIConnectionSet.cc
  src/MWBlobIO.cc
  src/MWConnection.cc
  src/MWConnectionSet.cc
  src/MWError.cc
  src/MWIos.cc
  src/MWMultiStep.cc
  src/MWSimpleStep.cc
  src/MWSolveStep.cc
  src/MWStep.cc
  src/MWStepFactory.cc
  src/MWStepVisitor.cc
  src/NodeDesc.cc
  src/ObsDomain.cc
  src/ParameterHandler.cc
  src/SocketConnection.cc
  src/SocketConnectionSet.cc
  src/SocketListener.cc
  src/VdsDesc.cc
  src/VdsPartDesc.cc
  src/WorkDomainSpec.cc
  src/WorkerControl.cc
  src/WorkerFactory.cc
  src/WorkerInfo.cc
  src/WorkerProxy.cc
  src/WorkersDesc.cc)
