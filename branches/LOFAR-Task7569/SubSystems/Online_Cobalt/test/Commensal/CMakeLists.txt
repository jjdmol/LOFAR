# $Id$

include(LofarCTest)

set(_tests
  tCommensal_3sec_2st_3sb
)

foreach(_test ${_tests})
  lofar_add_test(${_test})
  set_tests_properties(${_test} PROPERTIES RUN_SERIAL ON)
endforeach(_test ${_tests})
