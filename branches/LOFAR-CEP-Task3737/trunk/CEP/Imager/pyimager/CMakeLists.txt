# $Id$

lofar_package(pyimager 0.1 DEPENDS casaimwrap)

add_subdirectory(processors/gpu)


include(PythonInstall)
python_install(__init__.py
    processors/__init__.py
    processors/data_processor_base.py
    processors/data_processor_default.py
    processors/data_processor_low_level_base.py
    processors/casa/__init__.py
    processors/casa/data_processor.py
    processors/casa/data_processor_low_level.py
    processors/parallel/__init__.py
    processors/parallel/data_processor_parallel.py
    processors/parallel/data_processor_parallel_low_level.py
    processors/casa/visimagingweight.py
    processors/pywsplit/__init__.py
    processors/pywsplit/data_processor.py
    processors/pywsplit/data_processor_low_level.py
    processors/pywsplit/mod_threadpool.py
    algorithms/__init__.py
    algorithms/constants.py
    algorithms/empty.py
    algorithms/mfclean.py
    algorithms/util.py
    DESTINATION lofar/pyimager)

install(PROGRAMS pyimager.py DESTINATION bin)

