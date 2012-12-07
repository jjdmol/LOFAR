# $Id$

lofar_package(pyimager 0.1 DEPENDS casaimwrap)

include(PythonInstall)
python_install(__init__.py
    processors/__init__.py
    processors/casa/__init__.py
    processors/casa/data_processor.py
    processors/casa/data_processor_low_level.py
    processors/casa/data_processor_parallel.py
    processors/casa/data_processor_parallel_low_level.py
    algorithms/__init__.py
    algorithms/constants.py
    algorithms/empty.py
    algorithms/mfclean.py
    algorithms/util.py
    DESTINATION lofar/pyimager)

install(PROGRAMS pyimager.py DESTINATION bin)
