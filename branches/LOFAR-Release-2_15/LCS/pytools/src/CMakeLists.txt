# $Id$

include(LofarPackageVersion)

lofar_add_library(lofar_pytools
        Package__Version.cc
        PycExcp.cc
        PycBasicData.cc
)

lofar_add_bin_program(versionpytools versionpytools.cc)

# Install Python modules
include(PythonInstall)
python_install(__init__.py DESTINATION lofar)


