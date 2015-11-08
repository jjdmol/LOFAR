## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)
set(CTEST_PROJECT_NAME "LOFAR")
set(CTEST_NIGHTLY_START_TIME "01:00:00 CET")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "svn.astron.nl")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=LOFAR")
set(CTEST_DROP_SITE_CDASH TRUE)

# Decrease test timeout. Valgrind times are excessive and need much more.
set (CTEST_TEST_TIMEOUT 150 CACHE STRING 
    "Maximum ctest -S time allowed before CTest will kill the test." FORCE) 
# Already a cache var. It can be overridden using ctest --timeout N
set (DART_TESTING_TIMEOUT 150 CACHE STRING 
    "Maximum ctest time allowed before CTest will kill the test." FORCE)
