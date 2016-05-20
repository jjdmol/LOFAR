# $Id: CMakeLists.txt $
include(LofarCTest)

lofar_add_test(t_resource_estimator)

set(_py_files
    __init__.py
    testset.py
)

python_install(${_py_files} DESTINATION lofar/sas/resourceassignment/resourceassignmentestimator/test)
