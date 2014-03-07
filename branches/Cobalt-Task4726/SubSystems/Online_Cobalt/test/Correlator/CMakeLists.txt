# $Id$

include(LofarCTest)

set(_tests
  tCorrelate_1sec_1st_5sb_noflagging
  tCorrelate_3sec_1st_5sb
  tCorrelate_3sec_2st_5sb
)

foreach(_test ${_tests})
  lofar_add_test(${_test})
  set_tests_properties(${_test} PROPERTIES RUN_SERIAL ON)
endforeach(_test ${_tests})
