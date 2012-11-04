add_library(_pywcs MODULE
  wcslib/C/flexed/wcsbth.c
  wcslib/C/flexed/wcspih.c
  wcslib/C/flexed/wcsulex.c
  wcslib/C/flexed/wcsutrn.c
  wcslib/C/cel.c
  wcslib/C/lin.c
  wcslib/C/log.c
  wcslib/C/prj.c
  wcslib/C/spc.c
  wcslib/C/sph.c
  wcslib/C/spx.c
  wcslib/C/tab.c
  wcslib/C/wcs.c
  wcslib/C/wcserr.c
  wcslib/C/wcsfix.c
  wcslib/C/wcshdr.c
  wcslib/C/wcsprintf.c
  wcslib/C/wcsunits.c
  wcslib/C/wcsutil.c
  src/distortion.c
  src/distortion_wrap.c
  src/docstrings.c
  src/pipeline.c
  src/pyutil.c
  src/pywcs.c
  src/sip.c
  src/sip_wrap.c
  src/str_list_proxy.c
  src/util.c
  src/wcslib_wrap.c
  src/wcslib_tabprm_wrap.c
  src/wcslib_units_wrap.c
  src/wcslib_wtbarr_wrap.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/wcslib/C)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${NUMPY_INCLUDE_DIR})
add_definitions(-DECHO)
add_definitions(-DWCSTRIG_MACRO)
add_definitions(-DPYWCS_BUILD)
add_definitions(-DGNU_SOURCE)
add_definitions(-DNDEBUG)
add_definitions(-UDEBUG)

set_target_properties(_pywcs PROPERTIES PREFIX "")

if (APPLE)
  set_target_properties(_pywcs PROPERTIES
    LINK_FLAGS "-undefined dynamic_lookup")
endif (APPLE)

install(TARGETS _pywcs DESTINATION ${PYTHON_INSTALL_DIR}/lofar/bdsm)
install(FILES lib/pywcs/pywcs.py DESTINATION ${PYTHON_INSTALL_DIR}/lofar/bdsm)
install(FILES lib/pywcs/_docutil.py DESTINATION ${PYTHON_INSTALL_DIR}/lofar/bdsm)
