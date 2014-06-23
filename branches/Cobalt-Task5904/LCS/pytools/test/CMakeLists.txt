# $Id$

include(LofarCTest)

add_library(_tConvert MODULE tConvert.cc)
target_link_libraries(_tConvert lofar_pytools)
set_target_properties(_tConvert PROPERTIES 
  PREFIX "")

lofar_add_test(tConvert DEPENDS _tConvert)
