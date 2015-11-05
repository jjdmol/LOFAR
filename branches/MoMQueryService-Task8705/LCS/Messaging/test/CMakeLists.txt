# $Id: CMakeLists.txt 1457 2015-08-18 14:44:52Z loose $

include(LofarCTest)

set(_unit_tests 
  tLofarMessages
  tMessaging
)

lofar_find_package(UnitTest++)

if(UNITTEST++_FOUND)
  foreach(_test ${_unit_tests})
    lofar_add_test(${_test} ${_test}.cc)
  endforeach()
else()
  lofar_join_arguments(_unit_tests)
  message(WARNING "UnitTest++ not found. "
    "The following tests will not be run: ${_unit_tests}")
endif()
