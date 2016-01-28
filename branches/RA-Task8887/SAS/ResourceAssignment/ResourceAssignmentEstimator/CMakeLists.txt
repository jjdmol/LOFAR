# $Id: CMakeLists.txt  $

lofar_package(ResourceAssignmentEstimator 1.0 DEPENDS PyMessaging)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

set(_py_files
  __init__.py
  service.py
)

python_install(${_py_files} DESTINATION lofar/sas/resourceassignment/resourceassignmentestimator)

add_subdirectory(test)
add_subdirectory(resource_estimators)
