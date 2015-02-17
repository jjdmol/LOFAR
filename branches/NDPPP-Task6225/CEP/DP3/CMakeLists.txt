# $Id$

lofar_add_package(DPPP)
lofar_add_package(TestDynDPPP)
if(HAVE_PYTHON)
    lofar_add_package(PythonDPPP)
endif()
lofar_add_package(AOFlagger)
lofar_add_package(SPW_Combine SPWCombine)
