# $Id$

include(LofarCTest)

file(COPY
  ${CMAKE_CURRENT_SOURCE_DIR}/python-coverage.sh
  DESTINATION ${CMAKE_BINARY_DIR}/bin)

lofar_add_test(t_dbcredentials)
