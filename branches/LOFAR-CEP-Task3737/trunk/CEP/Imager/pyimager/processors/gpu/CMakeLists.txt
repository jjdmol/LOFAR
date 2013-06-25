# $Id$

include(PythonInstall)
python_install(
    __init__.py
    data_processor.py
    DESTINATION lofar/pyimager/processors/gpu/)
