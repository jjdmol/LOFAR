# $Id: CMakeLists.txt 33087 2015-12-03 08:24:17Z schaap $

lofar_package(XML_generator 1.0)

lofar_find_package(Python 2.6 REQUIRED)
include(PythonInstall)

python_install(
    xmlgen.py
    DESTINATION lofar/sas/xmlgenerator)

lofar_add_bin_scripts(
  xmlgen
)

