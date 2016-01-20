# $Id: CMakeLists.txt  $

set(_py_files
    __init__.py
    base_resource_estimator.py
    observation.py
    longbaseline_pipeline.py
    calibration_pipeline.py
    image_pipeline.py
    pulsar_pipeline.py
)

python_install(${_py_files} DESTINATION lofar/sas/resourceassignment/resourceassignmentestimator/resource_estimators)
