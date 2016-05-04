# $Id$

lofar_package(ObservationStartListener 1.0 DEPENDS MessageBus pyparameterset)

include(LofarFindPackage)
lofar_find_package(Python 2.6 REQUIRED)

# ! Python version detection fails on not so new systems. Best solution is an
# ! ugly hack: No use wasting more of our time on compat w/ ancient software.
# ! Python 2.4 is 10+ yrs old. Stop wasting time! Fix the real problem!!!
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; sys.exit(not(sys.version_info[0] >= 2 and sys.version_info[1] >= 6 and sys.version_info[2] >= 0))" RESULT_VARIABLE py260_not_supp)
if(NOT py260_not_supp)  # double negation also skips pkg if command fails
  lofar_find_package(QPID)
  lofar_find_package(LibXMLxx)

  add_subdirectory(etc)
  add_subdirectory(src)
  add_subdirectory(test)
else(NOT py260_not_supp)
  message(WARNING "package ObservationStartListener skipped: found python is too old (or detection cmd failed)")
endif(NOT py260_not_supp)
