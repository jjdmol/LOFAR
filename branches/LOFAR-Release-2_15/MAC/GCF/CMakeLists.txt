# $Id$

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
  ${CMAKE_BINARY_DIR}/include/GCF)

lofar_add_package(GCFTM   TM)     # Task Management
lofar_add_package(GCFPVSS PVSS)   # Low-level interface to PVSS database
lofar_add_package(GCFRTDB RTDB)   # Real-Time Database layer.
